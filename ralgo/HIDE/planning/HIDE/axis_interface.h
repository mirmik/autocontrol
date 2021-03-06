#ifndef RALGO_PLANNING_AXIS_INTERFACE_H
#define RALGO_PLANNING_AXIS_INTERFACE_H

#include <igris/util/bug.h>

#define NODTRACE 0
#include <igris/dtrace.h>
//#include <ralgo/virtdevs/device.h>

#include <ralgo/objects/named.h>


/**
	Интерфейс управления базовыми функциями осей.
	Самые простые, часто применяемые функции, которые можно
	добавить в прототип интерфейса для обеспечения минимального
	набора функционала.

	Реализацией axis_interface могут 
	быть axis_controller (18.11.2019 находится в planning/axis.h),
	который реализует логику траекторного расчёта фаз.

	Прочие реализации, управляющие непосредственно, или методом сообщений.
*/

namespace ralgo
{
	static constexpr int FORWARD = 1;
	static constexpr int BACKWARD = -1;

	enum axis_operation_status
	{
		STOPED,
		MOVED,
		CONTROLLED
	};

	template <class ExtPos, class IntPos = ExtPos,
	          class Speed = float, class Time = int64_t>
	class axis_interface : public virtual ralgo::named
	{
		float c_ext2int = 1;
		float c_int2ext = 1;

		float _gain = 1;
		bool _reverse = false;
		IntPos _forw = 1000;
		IntPos _back = -1000;

		Time _accdcc_protector = 1000;
		Time _accdcc = 4000;

		Speed _speed_protector = 1000;
		Speed _speed = 1;

	public:
		bool debug_mode = false;

		// JOG
		virtual int jog(int direction) { BUG(); }

		// INCMODE
		int incmove(ExtPos pos) { return incmove_internal(ext2int(pos)); }
		int incmove_internal(IntPos pos) { if (!position_in_limits(position() + pos)) return -1; return incmove_internal_unsafe(pos); }
		virtual int incmove_internal_unsafe(IntPos pos) { BUG(); }

		// ABSMOVE
		int absmove(ExtPos pos) { DTRACE(); return absmove_internal(ext2int(pos)); }
		int absmove_internal(IntPos pos) { DTRACE(); if (!position_in_limits(pos)) return -1; return absmove_internal_unsafe(pos); }
		virtual int absmove_internal_unsafe(IntPos pos) { BUG(); }

		// RELMOVE
		int relmove(ExtPos pos) { return relmove_internal(ext2int(pos)); }
		int relmove_internal(IntPos pos) { if (!position_in_limits(pos)) return -1; return absmove_internal_unsafe(pos); }
		virtual int relmove_internal_unsafe(IntPos pos) { BUG(); }

		// POSITION
		ExtPos position() { return int2ext(internal_position()); }
		virtual IntPos internal_position() { BUG(); }

		// STOP
		virtual int stop() { BUG(); }
		virtual int hardstop() { BUG(); }

		// HOME_POSITION
		virtual int set_home_position() { BUG(); }

		// LIMITS
		bool position_in_limits(ExtPos pos) { return DTRACE_RET(pos > _back && pos < _forw); }
		void set_limits(ExtPos back, ExtPos forw) { _back = ext2int(back); _forw = ext2int(forw); }

		void set_backward_limit(ExtPos back) { _back = ext2int(back); }
		void set_forward_limit (ExtPos forw) { _forw = ext2int(forw); }
		void set_backward_limit_internal(IntPos back) { _back = back; }
		void set_forward_limit_internal (IntPos forw) { _forw = forw; }

		ExtPos backward_limit() { return int2ext(_back); }
		ExtPos forwward_limit() { return int2ext(_forw); }
		IntPos backward_limit_internal() { return _back; }
		IntPos forwward_limit_internal() { return _forw; }

		// REVERSE
		void set_reverse(bool en) { _reverse = en; }
		bool reverse() { return _reverse; }

		// GAIN
		void set_gain(double gain) { c_ext2int = gain; c_int2ext = 1 / gain; }
		auto gain() { return c_ext2int; }
		IntPos ext2int(ExtPos e) { return e * c_ext2int * (reverse() ? -1 : 1); }
		ExtPos int2ext(IntPos i) { return i * c_int2ext * (reverse() ? -1 : 1); }

		// SPEED
		virtual void set_speed(Speed spd) { _speed = protect_speed(spd); }
		Speed setted_speed() { return _speed; }
		Speed internal_speed() { return ext2int(_speed); }
		Speed protect_speed(Speed spd) { if (spd > _speed_protector) return _speed_protector; else return spd; }

		// ACCDCC
		virtual void set_accdcc(Time accdcc) { _accdcc = protect_accdcc(accdcc); }
		Time setted_accdcc() { return _accdcc; }
		Time protect_accdcc(Time accdcc) { if (accdcc < _accdcc_protector) return _accdcc_protector; else return accdcc; }
	};
}

#endif