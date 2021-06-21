


#ifndef __LZRTAG_WEAPON_HANDLER_H__
#define __LZRTAG_WEAPON_HANDLER_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <xasin/audio.h>

#include <functional>

#undef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include <esp_log.h>

namespace LZRTag {
namespace Weapon {

enum wait_failure_t {
	TRIGGER_PRESSED,
	TIMEOUT,
	CANNOT_SHOOT,
	INVALID_CONFIG,
};

struct ammo_info_t {
	int32_t current_ammo;
	int32_t clipsize;
	int32_t  total_ammo;
};

class BaseWeapon;

class Handler {
protected:
friend BaseWeapon;

	Xasin::Audio::TX &audio;
	Xasin::Audio::Source * previous_source;
	Xasin::Audio::Source * current_source;

	// Current weapon points to the weapon instance that is actively being used.
	// target_weapon points to the weapon the user wants to use. If the pointers
	// are different, a weapon switch will be initiated.
	BaseWeapon *target_weapon;
	BaseWeapon *current_weapon;

	// FreeRTOS Task Handle pointing to the internal weapon shot management
	// thread. Mainly used for xTaskNotify to update on button press or
	// weapon change.
	TaskHandle_t process_task;

	// First tick at which an action (reloading, weapon switch,
	// forced weapon cooldown) occurs. Used for time-keeping.
	TickType_t  action_start_tick;

	TickType_t last_shot_tick;

	// Current status of the trigger button. True means "shoot"
	bool trigger_state;
	// "New" flag for the trigger button state. Allows weapons to only wait
	// for the initial button press, and no re-trigger until the button is released.
	bool trigger_state_read;

	float gun_heat;

public:
	Handler(Xasin::Audio::TX &audio);

	void _internal_run_thread();
	void start_thread();

	template<class T>
	Xasin::Audio::Source * play(const T &sample) {
		Xasin::Audio::Source * next_source = audio.play(sample, false);
		ESP_LOGD("LZR::WPN", "Trying to play 0x%p", next_source);
		
		if(next_source == nullptr) {
			return nullptr;
		}

		if(previous_source != nullptr) {
			previous_source->fade_out();
			previous_source->release();
		}
		previous_source = current_source;
		current_source = next_source;
		
		return current_source;
	}

	/*!	@brief Wait for the user to press the trigger
	 *	@details This function will wait for the user to press the trigger.
	 *		The optional parameters allow the weapon to require a re-press (having to release
	 *		and then press) as well as a timeout.
	 *	@param max_ticks Number of FreeRTOS ticks to wait, at most.
	 *  @param repress_required Whether or not the press has to be 'fresh', i.e. the user
	 * 		had released it at least once.
	 * 	@return True if the condition matched, false on timeout OR when the user stops
	 * 		wanting to shoot (by dying, weapon switch, etc.)
	 */
	wait_failure_t wait_for_trigger(TickType_t max_ticks = portMAX_DELAY, bool repress_needed = false);
	/*!	@brief Waits for the user to release the trigger
	 *	@details This function will wait for the user to release
	 *		the trigger, and will return true. It may also return
	 *		prematurely in case of another event, such as the weapon
	 *		shot being cancelled.
	 *	@param max_ticks Number of FreeRTOS ticks to wait
	 *	@return true if the user released the trigger, false if some other
	 *		event cancels the shot.
	 */
	wait_failure_t wait_for_trigger_release(TickType_t max_ticks = portMAX_DELAY);

	/*!	@brief Waits for the given number of FreeRTOS ticks
	 *	@details This function will wait for the given number of FreeRTOS
	 *		ticks. It may also return prematurely if the current shot
	 *		is being cancelled (can_shoot returns false).
	 *	@return true if this function could wait the given number of 
	 *		ticks, false if the wait was aborted.
	 */
	wait_failure_t wait_ticks(TickType_t ticks);

	void boop_thread();

	void update_btn(bool new_button_state);
	void fx_tick();

	float get_gun_heat();

	bool was_shot_tick();
	TickType_t get_last_shot_tick();

	std::function<void (void)> on_shot_func;

	bool get_btn_state(bool only_fresh = false);

	// Returns true for as long as the player can shoot, i.e. he is
	// alive, does not want to reload or change weapons, etc.
	//
	// Any weapon code MUST exit the weapon handling function ASAP when
	// this is false.
	bool can_shoot();

	std::function<bool (void)> can_shoot_func;

	/*! @brief Returns true if weapons have to use ammo but have
	 *  infinite clips.
	 */
	bool infinite_clips();
	/*! @brief Returns true if weapons should ignore ammo
	 *  alltogether.
	 */
	bool infinite_ammo();

	void tempt_reload();
	ammo_info_t get_ammo();

	void set_weapon(BaseWeapon *next_weapon);
	bool weapon_equipped();

	void apply_vibration(float &vibr);
};

}
}

#endif
