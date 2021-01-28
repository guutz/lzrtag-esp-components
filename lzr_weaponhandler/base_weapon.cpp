

#include <lzrtag/weapon.h>

namespace LZRTag {
namespace Weapon {

BaseWeapon::BaseWeapon(Handler &handler) 
	: handler(handler),
	equip_duration(2000 / portTICK_PERIOD_MS),
	wants_to_reload(false), reload_duration(1000 / portTICK_PERIOD_MS) {

}

bool BaseWeapon::can_shoot() {
	return false;
}
bool BaseWeapon::can_reload() {
	return false;
}

void BaseWeapon::reload_start() {}
void BaseWeapon::reload_tick() {}

void BaseWeapon::shot_process() {
	xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY);
}

int32_t BaseWeapon::get_clip_ammo() {
	return 1;
}
int32_t BaseWeapon::get_max_clip_ammo() {
	return 1;
}

int32_t BaseWeapon::get_total_ammo() {
	return 1;
}

void BaseWeapon::tempt_reload() {
	if(!can_reload())
		return;
	
	wants_to_reload = true;
	handler.boop_thread();
}

}
}