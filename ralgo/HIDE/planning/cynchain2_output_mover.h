#ifndef RALGO_CYNEMATIC_CHAIN_OBJECT2D_H
#define RALGO_CYNEMATIC_CHAIN_OBJECT2D_H

#include <ralgo/cynematic/link2d.h>
#include <ralgo/cynematic/chain2d.h>
#include <ralgo/planning/htrans2_mover.h>

#include <ralgo/planning/disctime.h>
#include <ralgo/util/backpack.h>

#include <nos/trace.h>
#include <nos/print.h>

#include <ralgo/objects/served.h>

namespace ralgo 
{
	class cynchain2_output_mover : public htrans2_mover, 
		virtual public ralgo::served
	{
	public:
		ralgo::cynematic_chain2d chain;
		float compensation_koefficient;

	public:

		rabbit::htrans2<float> control_position() 
		{
			return chain.out()->global_location;
		} 

		void setup(
			igris::array_view<ralgo::unit2d*> a,
			igris::array_view<ralgo::cynematic_unit2d*> b) 
		{
			chain.setup(a,b);
		}

		cynchain2_output_mover() 
		{
			compensation_koefficient = 1 / ralgo::discrete_time_frequency();
		}

		rabbit::htrans2<float> location() 
		{
			return chain.out()->global_location;
		}

		// Контроллер должен уметь восстанавливать текущее положение
		// контрлирующих элементов по реальному положению системы.
		virtual void restore_control_model() = 0; 

		void update_model_location() 
		{
			chain.update_model_location();
		}

		void collect_chain(unit2d* finallink, unit2d* startlink = nullptr) 
		{
			chain.collect_chain(finallink, startlink);			
		}

		void set_phase(rabbit::htrans2<float> pos, rabbit::screw2<float> spd) 
		{
			//TRACE();
			rabbit::screw2<float> senses[chain.pairs.size()];
			rabbit::screw2<double> dsenses[chain.pairs.size()];
			//(void) senses;
			double spdarr[chain.pairs.size()];
			memset(spdarr, 0, sizeof(spdarr));

			//Можно целевую позицию переводить в связный базис, а чувствительность
			// брать по связному базису.
			// Тогда вычитание не понадобится. 
			// И перевод чувствительности в основной базис не нужен будет тоже.
			chain.sensivity(senses, chain.chain[0]);
			rabbit::screw2<float> target = 
				spd + (pos - location()) * compensation_koefficient; 	

			//nos::println(pos, location(), pos - location(), target);

			rabbit::screw2<double> dtarget = target;

			for (int i = 0; i < chain.pairs.size(); ++i) 
			{
				dsenses[i] = senses[i];
			}

			// Поиск скоростей звеньев удовлетворяющих заданному
			// управлению.
			ralgo::svd_backpack<double, rabbit::screw2<double>>(
				spdarr, dtarget, 
				dsenses, chain.pairs.size());

			// Выставляем найденные скорости прилинкованным 
			// сервам.
			for (int i = 0; i < chain.pairs.size(); ++i) 
			{
				cynematic_unit2d * _unit = chain.pairs[i];
				unit2d_1dof * unit = (unit2d_1dof *) _unit;

				unit -> set_speed_for_linked(spdarr[i]);
			}
		}

		virtual void get_control_phase(int64_t time,
			rabbit::htrans2<float>& pos, rabbit::screw2<float>& spd) = 0;

		void serve() override
		{
			//TRACE();
			/*if (external_controller != nullptr 
				|| current_trajectory == nullptr) 
			{
				return;
			} */

			rabbit::htrans2<float> pos{}; 
			rabbit::screw2<float> spd{};

			get_control_phase(ralgo::discrete_time(), pos, spd);
			set_phase(pos, spd);	
		}

		void activate() override
		{
			restore_control_model();
		}

		void deactivate() override
		{} 
	};
}

#endif