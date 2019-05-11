registerNpc(273, {
  walk_speed        = 230,
  run_speed         = 850,
  scale             = 140,
  r_weapon          = 1016,
  l_weapon          = 0,
  level             = 65,
  hp                = 33,
  attack            = 306,
  hit               = 145,
  def               = 165,
  res               = 135,
  avoid             = 97,
  attack_spd        = 110,
  is_magic_damage   = 0,
  ai_type           = 328,
  give_exp          = 48,
  drop_type         = 228,
  drop_money        = 12,
  drop_item         = 46,
  union_number      = 46,
  need_summon_count = 0,
  sell_tab0         = 0,
  sell_tab1         = 0,
  sell_tab2         = 0,
  sell_tab3         = 0,
  can_target        = 0,
  attack_range      = 250,
  npc_type          = 3,
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