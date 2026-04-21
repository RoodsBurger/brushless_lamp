#include "Sensor.h"
#include "../foc_utils.h"
#include "../time_utils.h"



void Sensor::update() {
    float val = getSensorAngle();
    // PATCH (2.4.0): reject negative angles as error signals — some sensors return <0 on fault
    if (val < 0) return;
    angle_prev_ts = _micros();
    float d_angle = val - angle_prev;
    // if overflow happened track it as full rotation
    if(abs(d_angle) > (0.8f*_2PI) ) full_rotations += ( d_angle > 0 ) ? -1 : 1;
    angle_prev = val;
}


 /** get current angular velocity (rad/s) */
float Sensor::getVelocity() {
    // calculate sample time
    float Ts = (angle_prev_ts - vel_angle_prev_ts)*1e-6f;
    // PATCH (2.4.0): micros() overflow → Ts negative; reset baseline and return last value
    if (Ts < 0.0f) {
        vel_angle_prev = angle_prev;
        vel_full_rotations = full_rotations;
        vel_angle_prev_ts = angle_prev_ts;
        return velocity;
    }
    if (Ts < min_elapsed_time) return velocity; // don't update velocity if deltaT is too small

    // PATCH (2.4.0): avoid float precision loss at large full_rotations; only update when
    // angle changed meaningfully. Our POS_MAX=60 rad (~9.5 revs) is well within float
    // precision but this is still a correctness improvement.
    float current_angle, prev_angle;
    if (full_rotations == vel_full_rotations) {
        current_angle = angle_prev;
        prev_angle    = vel_angle_prev;
    } else {
        current_angle = (float)full_rotations * _2PI + angle_prev;
        prev_angle    = (float)vel_full_rotations * _2PI + vel_angle_prev;
    }
    const float delta_angle = current_angle - prev_angle;
    if (fabsf(delta_angle) > 1e-8f) {
        velocity = delta_angle / Ts;
        vel_angle_prev = angle_prev;
        vel_full_rotations = full_rotations;
        vel_angle_prev_ts = angle_prev_ts;
    }
    return velocity;
}



void Sensor::init() {
    // initialize all the internal variables of Sensor to ensure a "smooth" startup (without a 'jump' from zero)
    getSensorAngle(); // call once
    delayMicroseconds(1);
    vel_angle_prev = getSensorAngle(); // call again
    vel_angle_prev_ts = _micros();
    delay(1);
    getSensorAngle(); // call once
    delayMicroseconds(1);
    angle_prev = getSensorAngle(); // call again
    angle_prev_ts = _micros();
}


float Sensor::getMechanicalAngle() {
    return angle_prev;
}



float Sensor::getAngle(){
    return (float)full_rotations * _2PI + angle_prev;
}



double Sensor::getPreciseAngle() {
    return (double)full_rotations * (double)_2PI + (double)angle_prev;
}



int32_t Sensor::getFullRotations() {
    return full_rotations;
}



int Sensor::needsSearch() {
    return 0; // default false
}
