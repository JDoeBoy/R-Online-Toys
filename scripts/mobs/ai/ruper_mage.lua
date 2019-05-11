registerNpc(346, {
  walk_speed        = 200,
  run_speed         = 700,
  scale             = 140,
  r_weapon          = 1064,
  l_weapon          = 0,
  level             = 95,
  hp                = 27,
  attack            = 399,
  hit               = 213,
  def               = 301,
  res               = 346,
  avoid             = 143,
  attack_spd        = 95,
  is_magic_damage   = 1,
  ai_type           = 128,
  give_exp          = 77,
  drop_type         = 377,
  drop_money        = 10,
  drop_item         = 48,
  union_number      = 48,
  need_summon_count = 0,
  sell_tab0         = 0,
  sell_tab1         = 0,
  sell_tab2         = 0,
  sell_tab3         = 0,
  can_target        = 0,
  attack_range      = 1200,
  npc_type          = 7,
  hit_material_type = 1,
  face_icon         = 0,
  summon_mob_type   = 0,
  quest_type        = 0,
  height            = 0
});

function OnInit(entity)
  return true
end

function OnCreate(entity)
  return true
end

function OnDelete(entity)
  return true
end

function OnDead(entity)
end

function OnDamaged(entity)
end