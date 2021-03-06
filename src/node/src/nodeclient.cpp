// Copyright 2016 Chirstopher Torres (Raven), L3nn0x
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "connection.h"
#include "cnetwork_asio.h"
#include "nodeclient.h"
#include "packetfactory.h"
#include "croseserver.h"
#include "epackettype.h"
#include "config.h"
#include "nodesessions.h"

using namespace RoseCommon;

NodeClient::NodeClient()
  : CRoseClient(),
    session_id_( 0 ),
    buffered_packet_(nullptr){}

NodeClient::NodeClient(std::unique_ptr<Core::INetwork> _sock)
  : CRoseClient( move( _sock ) ),
    session_id_( 0 ),
    buffered_packet_(nullptr) {
      socket_[SocketType::Client]->registerOnShutdown(std::bind(&NodeClient::onShutdown, this));
    }

NodeClient::~NodeClient() {
  for( unsigned int i = 1; i < SocketType::MaxSockets; ++i )
  {
    if(socket_[i] != nullptr)
    {
      socket_[i]->shutdown(true);
      socket_[i]->set_active(false);
    }
  }
}

//---------------------------------
// SERVER PACKETS
bool NodeClient::serverAcceptReply([[maybe_unused]] Packet::SrvAcceptReply&& P) {
  logger_->trace( "NodeClient::serverAcceptReply start" );

#ifdef DYNAMIC_CRYPT
  crypt_[SocketType::CurrentMap].changeSeed(P.get_randValue());
#endif

  // Send the login packet to the server
  if(buffered_packet_ != nullptr)
  {
    send(*(buffered_packet_.get()), SocketType::CurrentMap);
    buffered_packet_ = nullptr;
  }
  return true;
}

bool NodeClient::serverSelectReply(Packet::SrvSrvSelectReply&& P) {
  logger_->trace( "NodeClient::serverSelectReply start" );
  auto& config = Core::Config::getInstance();
  
  auto conn = connectionPoolMem.getConnection<NodeDB>();
  NodeSessionsTable table{};
  
  conn(insert_into(table).set(table.id = P.get_sessionId(), table.name = get_name(), table.state = 1, table.charip = P.get_ip(), table.charport = P.get_port()));
  session_id_ = P.get_sessionId();
  
  auto packet = Packet::SrvSrvSelectReply::create(
    P.get_result(), P.get_sessionId(), P.get_cryptVal(),
    config.serverData().externalIp, config.charServer().clientPort); // Replace this with MY current ip address
    
  // Tell the client to connect to me!
  send(packet);
  return true;
}

bool NodeClient::serverSwitchServer(Packet::SrvSwitchServer&& P) {
  logger_->trace( "NodeClient::serverSwitchServer start" );
  auto& config = Core::Config::getInstance();
  
  auto conn = connectionPoolMem.getConnection<NodeDB>();
  NodeSessionsTable table{};
  conn(update(table).set(table.state = 2, table.worldip = P.get_ip(), table.worldport = P.get_port()).where(table.id == P.get_sessionId()));
  session_id_ = P.get_sessionId();
  
  auto packet = Packet::SrvSwitchServer::create(
    config.mapServer().clientPort, P.get_sessionId(), P.get_sessionSeed(), config.serverData().externalIp );

  // Tell the client to connect to me!
  send(packet);
  return true;
}

bool NodeClient::serverChangeCharReply(Packet::SrvChanCharReply&& P) {
  logger_->trace( "NodeClient::serverChangeCharReply start" );
  
  auto conn = connectionPoolMem.getConnection<NodeDB>();
  NodeSessionsTable table{};
  conn(update(table).set(table.state = 1).where(table.id == session_id_));
  
  // Let the client know they are allowed to change.
  send(P);
  socket_[SocketType::CurrentMap]->shutdown(true);
  socket_[SocketType::CurrentMap]->set_active(false);
  shutdown(true);
  return true;
}

bool NodeClient::serverLogoutReply(Packet::SrvLogoutReply&& P) {
  logger_->trace( "NodeClient::serverLogoutReply start" );
  
  // Let the client know the reply from the server before dcing them.
  send(P);
  
  if(P.get_waitTime() <= 0)
  {
    socket_[SocketType::CurrentMap]->shutdown(true);
    socket_[SocketType::CurrentMap]->set_active(false);
    shutdown(true);
  }
  return true;
}

//---------------------------------
// CLIENT PACKETS
bool NodeClient::clientAcceptReq([[maybe_unused]] Packet::CliAcceptReq&& P) {
  logger_->trace( "NodeClient::clientAcceptReq start" );
  
  auto cryptKey = std::time(nullptr);
  auto packet = Packet::SrvAcceptReply::create(Packet::SrvAcceptReply::ACCEPTED, cryptKey);
  send(packet);
  
#ifdef DYNAMIC_CRYPT
  // This may need to move to OnRecv to init the table AFTER the packet above is sent
  crypt_[SocketType::Client].changeSeed(cryptKey);
#endif

  return true;
}

bool NodeClient::clientLoginReq(Packet::CliLoginReq&& P) {
  logger_->trace( "NodeClient::clientLoginReq start" );
  auto& config = Core::Config::getInstance();

  disconnect(RoseCommon::SocketType::CurrentMap);
  init(config.nodeServer().loginIp, config.nodeServer().loginPort, RoseCommon::SocketType::CurrentMap);
  connect(RoseCommon::SocketType::CurrentMap);
  start_recv(RoseCommon::SocketType::CurrentMap);
  
  auto packet = Packet::CliAcceptReq::create();
  send(packet, SocketType::CurrentMap);
  
  auto username_ = Core::escapeData(P.get_username());
  set_name(username_);
  buffered_packet_ = std::make_unique<Packet::CliLoginReq>(Packet::CliLoginReq::create(P.getPacked().get()));
  return true;
}

bool NodeClient::clientJoinServerReq(Packet::CliJoinServerReq&& P) {
  logger_->trace( "NodeClient::clientJoinServerReq start" );
  
  auto conn = connectionPoolMem.getConnection<NodeDB>();
  NodeSessionsTable table{};

  session_id_ = P.get_sessionId();
  const auto res = conn(sqlpp::select(table.id, table.name, table.state, table.charip, table.charport, table.worldip, table.worldport).from(table).where(table.id == P.get_sessionId()));
  if (!res.empty()) {
    const auto &row = res.front();
    logger_->debug("id: '{}' name: '{}' state: '{}' charip: '{}' charport: '{}' worldip: '{}' worldport: '{}'", row.id, row.name, row.state, row.charip, row.charport, row.worldip, row.worldport);
    
    std::string ip = "";
    uint16_t port = 0;
    switch((uint16_t)row.state) {
      case 0: // Login server
      case 1: // Character server
      {
        ip = row.charip;
        port = row.charport;
        break;
      }
      case 2: // Map server
      {
        ip = row.worldip;
        port = row.worldport;
        break;
      }
      default:
        return false;
    }
    disconnect(RoseCommon::SocketType::CurrentMap);
    set_socket(std::make_unique<Core::CNetwork_Asio>(), RoseCommon::SocketType::CurrentMap, true);
    init(ip, port, RoseCommon::SocketType::CurrentMap);
    connect(RoseCommon::SocketType::CurrentMap);
    start_recv(RoseCommon::SocketType::CurrentMap);
    
    auto packet = Packet::CliAcceptReq::create();
    send(packet, SocketType::CurrentMap);
    buffered_packet_ = std::make_unique<Packet::CliJoinServerReq>(Packet::CliJoinServerReq::create(P.getPacked().get()));
  } else {
    logger_->warn("Lul, this guy!!! He's trying to join without logging in first! IP: {}", get_address());
    return false;
  }
  return true;
}


//---------------------------------
// MISC FUNCTIONS
bool NodeClient::handlePacket(uint8_t* _buffer) {
  logger_->trace( "NodeClient::HandlePacket start" );
  switch ( CRosePacket::type( _buffer ) ) {
    case ePacketType::PAKCS_ACCEPT_REQ:
      return clientAcceptReq(
          Packet::CliAcceptReq::create(_buffer));
    case ePacketType::PAKCS_LOGIN_REQ:
      return clientLoginReq(
          Packet::CliLoginReq::create(_buffer));
          
    // Character/Map server
    case ePacketType::PAKCS_JOIN_SERVER_REQ:
      return clientJoinServerReq(
          Packet::CliJoinServerReq::create(_buffer));
    default:
    {
      //TODO:: Make sure the socket is connected before doing this
      // Send the packet to the server
      auto res = std::make_unique<uint8_t[]>( CRosePacket::size(_buffer) );
      std::memcpy(res.get(), _buffer, CRosePacket::size(_buffer));
      send(std::move(res), SocketType::CurrentMap);
      return true;
    }
  }
}

bool NodeClient::handleServerPacket(uint8_t* _buffer) {
  logger_->trace( "NodeClient::HandleServerPacket start" );
  switch ( CRosePacket::type( _buffer ) ) {
    case ePacketType::PAKSS_ACCEPT_REPLY:
      return serverAcceptReply(
          Packet::SrvAcceptReply::create(_buffer));
    case ePacketType::PAKLC_SRV_SELECT_REPLY:
      return serverSelectReply(
        Packet::SrvSrvSelectReply::create(_buffer));
    case ePacketType::PAKCC_SWITCH_SERVER:
      return serverSwitchServer(
        Packet::SrvSwitchServer::create(_buffer));
    case ePacketType::PAKCC_CHAN_CHAR_REPLY:
      return serverChangeCharReply(
        Packet::SrvChanCharReply::create(_buffer));
    case ePacketType::PAKWC_LOGOUT_REPLY:
      return serverLogoutReply(
        Packet::SrvLogoutReply::create(_buffer));
    default:
    {
      auto res = std::make_unique<uint8_t[]>( CRosePacket::size(_buffer) );
      std::memcpy(res.get(), _buffer, CRosePacket::size(_buffer));
      send( std::move(res), SocketType::Client );
      return true;
    }
  }
  return true;
}

bool NodeClient::onShutdown() {
  logger_->trace("NodeClient::onShutdown()");
  set_active(false);
  for( unsigned int i = 1; i < SocketType::MaxSockets; ++i )
  {
    if(socket_[i] != nullptr)
    {
      socket_[i]->shutdown(true);
      socket_[i]->set_active(false);
    }
  }
  return true;
}