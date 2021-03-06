#ifndef RALGO_SPEED_DEFORMATION_H
#define RALGO_SPEED_DEFORMATION_H

/**
	Деформаторы скорости для траекторных проходов.

	Деформаторы скорости нужны для того, чтобы наложить скоростной паттерн на
	траекторию описываемую уравнением s(t).

	Деформатор скорости D следует применять как:
	s = s(D_pos(t / tfull) * t)
	x = D_spd(t / tfull) \dot{s}(D_pos(t / tfull) * t)
*/

namespace ralgo
{
	class speed_deformer
	{
		float acc = 0;
		float dcc = 0;
		float f_time = 1;

		float strt_spd = 0;
		float fini_spd = 0;
		float real_spd = 0;

		float fini_acc_pos = 0;
		float strt_dcc_pos = 0;

	public:
		speed_deformer() {}

		// Вариант движения с увеличением максимальной скорости.
		void reset(float acc, float dcc, float sspd = 0, float fspd = 0)
		{
			this->acc = acc;
			this->dcc = dcc;
			this->f_time = 1;

			strt_spd = sspd;
			fini_spd = fspd;

			// Формула выводится из равенства площадей под идеальной и реальной кривыми.
			real_spd =
			    (f_time - strt_spd * acc / 2 - fini_spd * dcc / 2) /
			    (f_time - acc / 2 - dcc / 2);

			//Вычисляем коэффициенты позиции в точках окончания участков.
			fini_acc_pos = (strt_spd + real_spd) * acc / 2;
			strt_dcc_pos = fini_acc_pos + real_spd * (f_time - acc - dcc);
		}

		// Второй вариант инициализации, когда вместо увеличения максимальной скорости
		// расширяется время довода изделия.
		void reset2(float acc, float dcc, float sspd = 0, float fspd = 0)
		{
			this->acc = acc;
			this->dcc = dcc;

			strt_spd = sspd;
			fini_spd = fspd;
			real_spd = 1;

			if (acc + dcc < 2) 
			{
				f_time = 1 
					+ (1 - sspd) * acc / 2
					+ (1 - fspd) * dcc / 2;	
			}
			else 
			{
				real_spd = sqrt(2 / (acc + dcc));
				this->acc = acc * real_spd;
				this->dcc = dcc * real_spd;	
				f_time = this->acc + this->dcc;
			}

			//Вычисляем коэффициенты позиции в точках окончания участков.
			fini_acc_pos = (strt_spd + real_spd) * this->acc / 2;
			strt_dcc_pos = fini_acc_pos + real_spd 
				* (f_time - this->acc - this->dcc);

//			exit(0);
		}

		speed_deformer& operator = (const speed_deformer& oth) = default;

		speed_deformer(float acc, float dcc, float sspd = 0, float fspd = 0)
			: acc(acc), dcc(dcc), strt_spd(sspd), fini_spd(fspd)
		{
			reset(acc, dcc, sspd, fspd);
		}

		float posmod(float t)
		{
			if (t >= f_time)
			{
				//dprln("pa");
				return 1;
			}

			if (t < acc)
			{
				//dprln("pb");
				return
				    t * strt_spd
				    + t * (t / acc * (real_spd - strt_spd) / 2);
			}

			if (t < f_time - dcc)
			{
				//dprln("pc");
				return
				    fini_acc_pos
				    + real_spd * (t - acc);
			}

			else
			{
				//dprln("pd");
				auto loct = t - f_time + dcc;
				return strt_dcc_pos
				       + (loct) * real_spd
				       - (loct) * ((loct) / dcc * (real_spd - fini_spd)) / 2;
			}
		}

		float spdmod(float t)
		{
			//DPRINT(t);
			//DPRINT(f_time);
			if (t >= f_time)
			{
				//dprln("a");
				return fini_spd;
			}

			if (t < acc)
			{
				//dprln("b");
				float k = t / acc;
				return strt_spd * (1 - k) + real_spd * k;
			}

			else if (t < f_time - dcc)
			{
				//dprln("c");
				return real_spd;
			}

			else
			{
				//dprln("d");
				float k = (f_time - t) / dcc;
				return fini_spd * (1 - k) + real_spd * k;
			}
		}

		void nullify()
		{
			strt_spd = fini_spd = 0;
			f_time = 1;
		}

		void set_stop_pattern() 
		{
			acc = 0;
			dcc = 2;
			f_time = 2;

			strt_spd = 1;
			fini_spd = 0;
			real_spd = 1;

			fini_acc_pos = 0;
			strt_dcc_pos = 0;
		}

		static void acc_dcc_balance(float& acc, float& dcc)
		{
			float sum = acc + dcc;

			if (sum > 1.0)
			{
				acc = acc / sum;
				dcc = dcc / sum;
			}
		}
	};

	/*class speed_deformed
	{
		int64_t stim, ftim;

	public:
		ralgo::speed_deformer spddeform;

		speed_deformed(){}

		speed_deformed(int64_t stim, int64_t ftim) :
			stim(stim), ftim(ftim)
		{}
	};*/
}

#endif