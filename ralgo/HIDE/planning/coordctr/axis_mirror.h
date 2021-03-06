#ifndef RALGO_PLANNING_AXIS_MIRROR_H
#define RALGO_PLANNING_AXIS_MIRROR_H

#include <ralgo/planning/axis.h>

namespace ralgo 
{
	class axis_ofsetter : 
		public axis_controller<float, float>,
		public ralgo::served
	{
		ralgo::axis_controller<float,float> * followed;
		ralgo::phase_driver * drv;

	public:
		float mirror_add = 0;
		float mirror_mul = 1;

	public:
		ralgo::axis_controller<float,float> ctr;

		axis_ofsetter(const char* name) 
		{
			this->set_name(name);
			ctr.set_name(name, "ctr");
		}

		void serve() 
		{
			auto time = ralgo::discrete_time();

			float followed_pos;
			float followed_spd;

			float pos;
			float spd;

			followed->attime(time, followed_pos, followed_spd);
			ctr.attime(time, pos, spd);

			followed_pos = followed_pos * mirror_mul + mirror_add;
			followed_spd = followed_spd * mirror_mul;

			drv->set_phase(
				pos + followed_pos,
				spd + followed_spd);
		}

		void set_driver(ralgo::phase_driver * drv) 
		{
			this->drv = drv;
		}

		void set_target(ralgo::axis_controller<float,float> * tgt) 
		{
			this->followed = tgt;
		}

		void restore_control_model() 
		{
			auto realpos = drv->current_position();
			auto mirrpos = followed->current_position();

			mirrpos = mirrpos / mirror_mul - mirror_add;

			ctr.set_current_position(realpos - mirrpos);
		}

		void activate() override { restore_control_model(); }
		void deactivate() override {}
	};
}

#endif