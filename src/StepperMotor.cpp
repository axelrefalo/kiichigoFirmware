#include "StepperMotor.h"
#include "Log.h"
#include <cmath>


StepperMotor::StepperMotor (std::string label,
                            StepperDriver &driver,
                            uint32_t steps_per_rev,
                            double default_speed) : 
                            label_(label),
                            driver_(driver)
                            {
    //Set default values 

    StepperDriver::StepMode step_mode = driver_.get_stepMode();
    auto modeMultiplier = STEP_MODE_MULTIPLIER[static_cast<size_t>(step_mode)];
    steps_per_rev_ = steps_per_rev*modeMultiplier;

    set_speed(default_speed);
    update_position();

    LOG_DEBUG("%s Defaults set: StepMode Multiplier- %d, steps per revolution- %d, \n", label_.c_str(), modeMultiplier, steps_per_rev_);


}

void StepperMotor::revolve(double revolutions){
    
    bool direction = revolutions>=0 ? true:false;
    driver_.set_direction(direction);
    
    LOG_DEBUG("%s direction set to: %s\n", label_.c_str(), direction ? "CW" : "CCW");
    
    uint32_t steps = static_cast<uint32_t>(std::round(std::abs(revolutions) * steps_per_rev_));
    driver_.step_for(steps);
    
    LOG_DEBUG("%s set for: %.2f revolutions\n", label_.c_str(), std::abs(revolutions));

}

void StepperMotor::set_speed(double rpm){
    speed_ = rpm;

    uint8_t pulse_width = driver_.get_pulse_width();
    double pulse_length = 60'000'000.0/(rpm* static_cast<double>(steps_per_rev_));
    uint32_t pulse_interval = static_cast<uint32_t>(std::round(pulse_length)) - pulse_width;

    driver_.set_pulse_interval(pulse_interval);

    LOG_DEBUG("%s Speed set to: %0.2f rpm(%d us pulse inteval)\n", label_.c_str(), rpm, pulse_interval);   

}

bool StepperMotor::get_direction(){
    return driver_.get_direction();
}

std::tuple<int32_t, double> StepperMotor::update_position(){
    position_step_ = driver_.get_step_tracker();
    position_revolutions_ = static_cast<double>(position_step_)/static_cast<double>(steps_per_rev_);

    LOG_DEBUG("%s Position: %d steps (%.2f revolutions) \n", label_.c_str(), position_step_, position_revolutions_);

    return std::make_tuple(position_step_,position_revolutions_);  
}

void StepperMotor::update_position(double rev_pos){

    position_step_ = static_cast<int32_t>(std::round(rev_pos * static_cast<double>(steps_per_rev_)));
    position_revolutions_ = static_cast<double>(position_step_)/static_cast<double>(steps_per_rev_);

    LOG_DEBUG("%s Position: %d steps (%.2f revolutions) \n", label_.c_str(), position_step_, position_revolutions_);
}

void StepperMotor::home(){

    driver_.home();
    update_position();

    LOG_DEBUG("%s HOME HIT! Position reset to: %d steps (%.2f revolutions) \n", label_.c_str(), position_step_, position_revolutions_);   

}

void StepperMotor::set_standbyMode(bool active){
    driver_.set_standbyMode(active);
    LOG_DEBUG("%s Standby Mode %s", label_.c_str(), active ?"enabled":"disabled");   

}

bool StepperMotor::active(){
    return driver_.active();
}

bool StepperMotor::step(){
    bool pulse_flag = driver_.step_pulse();
    
    if (pulse_flag){
        update_position();
    }

    return pulse_flag;
}

const std::string& StepperMotor::label() const{
    return label_;
};