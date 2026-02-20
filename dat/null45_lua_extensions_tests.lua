--#-package:0454ce20# -- package signature --
-- Any Lua files that depend on this file must have higher package numbers.

-- The following code is used to test the lua_extensions functions.
-- It also serves as examples of their use.

function null45_lua_extensions_test_dbpf_get_cohort_property_value()  
  -- Load the Exemplar Name property from 0x05342861 0x67BDDF0C 0x00000001.
  -- The group_id, instance_id, and property_id values can be either a number
  -- or a hexadecimal string.
  local group_id = '67bddf0c'
  local instance_id = 1
  local property_id = 32
  
  return dbpf.get_cohort_property_value(group_id, instance_id, property_id)
end

function null45_lua_extensions_test_dbpf_get_exemplar_property_value()
  -- Load the MaxTerrainHeight property from 0x6534284A 0x88CD66E9 0x00000001.
  -- The group_id, instance_id, and property_id values can be either a number
  -- or a hexadecimal string.
  local group_id = '88cd66e9'
  local instance_id = 1
  local property_id = '0x48cd7b26'
  
  return "MaxTerrainHeight=" .. tostring(dbpf.get_exemplar_property_value(group_id, instance_id, property_id))
end

function null45_lua_extensions_test_dbpf_resource_exists()  
  -- 0xEA5118B0 0xEA5118B1 0x00000001 is the Effect Directory in SimCity_1.dat.
  -- The type_id, group_id, and instance_id values can be either a number
  -- or a hexadecimal string.
  local type_id = 'ea5118b0'
  local group_id = 'ea5118b1'
  local instance_id = 1
  
  return tostring(dbpf.resource_exists(type_id, group_id, instance_id))
end

function null45_lua_extensions_test_execute_cheat()
  return tostring(game.execute_cheat('moolah 1000000000'))
end

function null45_lua_extensions_test_execute_command()
  return tostring(game.execute_cheat('GetPopupModalDialogsEnabled'))
end

function null45_lua_extensions_test_pause()
  game.pause()
  return "true"
end

function null45_lua_extensions_test_resume()
  game.resume()
  return "true"
end

function null45_lua_extensions_test_print()
  print("null45_lua_extensions_test_print")
  return "true"
end

function null45_lua_extensions_test_get_budget_department_total_expense()
  -- Get the total expense from the police budget department.
  -- The department_guid can be either a number or a hexadecimal string.
  local department_guid = 'a2963983'
  
  return tostring(sc4game.budget.get_department_total_expense(department_guid))
end

function null45_lua_extensions_test_get_budget_department_total_income()
  -- Get the total income from the police budget department.
  -- The department_guid can be either a number or a hexadecimal string.
  local department_guid = hex2dec('a2963983')
  
  return tostring(sc4game.budget.get_department_total_income(department_guid))
end

-- sc4game.language functions

function null45_lua_extensions_test_is_left_hand_drive()
  if sc4game.language.is_left_hand_drive() then
    return "Left"
  else
    return "Right"
  end  
end

function null45_lua_extensions_test_are_first_and_last_names_reversed()
  return tostring(sc4game.language.are_first_and_last_names_reversed())
end

function null45_lua_extensions_test_get_currency_symbol()
  return sc4game.language.get_currency_symbol()
end

function null45_lua_extensions_test_does_currency_symbol_precede_amount()
  return tostring(sc4game.language.does_currency_symbol_precede_amount())
end

function null45_lua_extensions_test_is_space_between_currency_symbol_and_amount()
  return tostring(sc4game.language.is_space_between_currency_symbol_and_amount())
end

function null45_lua_extensions_test_get_decimal_separator()
  return sc4game.language.get_decimal_separator()
end

function null45_lua_extensions_test_get_thousands_separator()
  return sc4game.language.get_thousands_separator()
end

function null45_lua_extensions_test_get_date_string()
  return sc4game.language.get_date_string(1, 1, 2000)
end

function null45_lua_extensions_test_get_money_string()
  return sc4game.language.get_money_string(123.456789)
end

function null45_lua_extensions_test_get_formatted_number_string()
  return sc4game.language.get_formatted_number_string(123.456789, 2)
end

function null45_lua_extensions_test_get_number_string()
  return sc4game.language.get_number_string(123.456789)
end

-- sc4game.city functions

function null45_lua_extensions_test_get_set_city_name()
  local city_name = sc4game.city.get_city_name()  
  sc4game.city.set_city_name(city_name)
  return city_name
end

function null45_lua_extensions_test_get_set_mayor_name()
  local mayor_name = sc4game.city.get_mayor_name()  
  sc4game.city.set_mayor_name(mayor_name)
  return mayor_name
end

function null45_lua_extensions_test_place_lot()
  local cell_x = 50  
  local cell_z = 50
  local orientation = sc4game_city_place_lot_orientation.NORTH
  -- The lot_exemplar_id and building_exemplar_id values can be either a number
  -- or a hexadecimal string.
  local lot_exemplar_id = hex2dec('60000462')
  local building_exemplar_id = '6070000'    
  
  return tostring(sc4game.city.place_lot(cell_x, cell_z, orientation, lot_exemplar_id, building_exemplar_id))
end

-- sc4game.camera functions

function null45_lua_extensions_test_get_set_camera_current_cell()
  local cell_x, cell_z = sc4game.camera.get_current_cell()  
  sc4game.camera.jump_to_cell(cell_x, cell_z)
  return "cell x=" .. tostring(cell_x) .. ", z=" .. tostring(cell_z)
end

function null45_lua_extensions_test_get_set_camera_current_position()
  local x, y, z = sc4game.camera.get_current_position()  
  sc4game.camera.jump_to_position(x, y, z)
  return "x=" .. tostring(x) .. ", y=" .. tostring(y) .. ", z=" .. tostring(z)
end

function null45_lua_extensions_test_camera_rotation()
  sc4game.camera.rotate_clockwise()  
  sc4game.camera.rotate_counter_clockwise()
  return "true"
end

function null45_lua_extensions_test_camera_take_snapshot_1()
  -- The path is the game's snapshot albums directory, typically Documents\SimCity 4\Albums.
  -- SC4 adds a \ to the end of the path before returning it, so we don't need to add one ourselves.
  local path = game.execute_cheat('GetDirectory Album') .. 'FullSnapshot.jpg'     
  return tostring(sc4game.camera.take_snapshot(path))
end

function null45_lua_extensions_test_camera_take_snapshot_2()
  local path = game.execute_cheat('GetDirectory Album') .. 'CroppedSnapshot.jpg'
  local area_x = 25
  local area_x = 25
  local area_width = 250
  local area_height = 250     
  return tostring(sc4game.camera.take_snapshot(path, area_x, area_y, area_width, area_height))
end

