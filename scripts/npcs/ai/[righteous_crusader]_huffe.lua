registerNpc(1111, {
  walk_speed        = 0,
  run_speed         = 0,
  scale             = 120,
  r_weapon          = 0,
  l_weapon          = 0,
  level             = 10,
  hp                = 100,
  attack            = 100,
  hit               = 100,
  def               = 100,
  res               = 100,
  avoid             = 100,
  attack_spd        = 100,
  is_magic_damage   = 0,
  ai_type           = 229,
  give_exp          = 0,
  drop_type         = 55,
  drop_money        = 0,
  drop_item         = 3,
  union_number      = 3,
  need_summon_count = 456,
  sell_tab0         = 456,
  sell_tab1         = 0,
  sell_tab2         = 0,
  sell_tab3         = 0,
  can_target        = 0,
  attack_range      = 200,
  npc_type          = 999,
  hit_material_type = 0,
  face_icon         = 16,
  summon_mob_type   = 16,
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