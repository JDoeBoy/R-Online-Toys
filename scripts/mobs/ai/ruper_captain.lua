registerNpc(348, {
  walk_speed        = 215,
  run_speed         = 700,
  scale             = 150,
  r_weapon          = 1091,
  l_weapon          = 0,
  level             = 100,
  hp                = 52,
  attack            = 533,
  hit               = 252,
  def               = 582,
  res               = 227,
  avoid             = 127,
  attack_spd        = 110,
  is_magic_damage   = 1,
  ai_type           = 129,
  give_exp          = 154,
  drop_type         = 380,
  drop_money        = 0,
  drop_item         = 53,
  union_number      = 53,
  need_summon_count = 0,
  sell_tab0         = 0,
  sell_tab1         = 0,
  sell_tab2         = 0,
  sell_tab3         = 0,
  can_target        = 0,
  attack_range      = 1600,
  npc_type          = 9,
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