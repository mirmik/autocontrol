#ifndef MADGWICK_AHRS_H_
#define MADGWICK_AHRS_H_

#include <linalg/linalg.h>
#include <math.h>

#include <igris/math/fast_invsqrt.h>
#include <igris/math/defs.h>

#define RAD_TO_DEG 180.0 / M_PI
#define SAMPLE_FREQ 1000.0f     // sample frequency in Hz
#define BETA_DEF    0.5f        // 2 * proportional gain

namespace ralgo
{
    class madgwick
    {

    public:
        madgwick();

        const linalg::vec<float,4>& quat() { return q; };
        
        void reset();

        void setKoeff(float sampleFreq, float beta);

        void update(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
        void update(float gx, float gy, float gz, float ax, float ay, float az);
        void update(float gx, float gy, float gz);

        float getPitchRad();
        float getRollRad();
        float getYawRad();
        float getPitchDeg();
        float getRollDeg();
        float getYawDeg();

        void ZYX(float *z, float *y, float *x);
        void ZYZ(float *z, float *y, float *z2);
        void ZYZ_u(float *z, float *y, float *z2);

    private:
        static float invSqrt(float x);

        volatile float beta;                // algorithm gain
        volatile float sampleFreq;
        volatile float invSampleFreq;

        linalg::vec<float,4> q;

        //volatile float q0 = 1.0f;
        //volatile float q1 = 0.0f;
        //volatile float q2 = 0.0f;
        //volatile float q3 = 0.0f;
    };
}

#endif