/*
 * Copyright (c) 2015 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef M2MREPORTHANDLER_H
#define M2MREPORTHANDLER_H

// Support for std args
#include <stdint.h>
#include "mbed-client/m2mconfig.h"
#include "mbed-client/m2mbase.h"
#include "mbed-client/m2mtimerobserver.h"

//algorithm can accept any number of limit values and report when signal changes between limit bands
#define MAX_LIMITS 2

//FORWARD DECLARATION
class M2MReportObserver;
class M2MTimer;

/**
 *  @brief M2MReportHandler.
 *  This class is handles all the observation related operations.
 */

class M2MReportHandler: public M2MTimerObserver
{
private:
    // Prevents the use of assignment operator by accident.
    M2MReportHandler& operator=( const M2MReportHandler& /*other*/ );

public:

    M2MReportHandler(M2MReportObserver &observer);

public:

    /**
     * Destructor
     */
    virtual ~M2MReportHandler();

    /**
     * @brief Sets that object is under observation.
     * @param Value for the observation.
     * @param handler, Handler object for sending
     * observation callbacks.
     */
    virtual void set_under_observation(bool observed);

    /**
     * @brief Sets the value of the given resource.
     * @param value, Value of the observed resource.
     */
    virtual void set_value(float value);

    /**
     * @brief Triggers object or object instance notification.
     */
    void trigger_object_notification();

    /**
     * @brief Parses the received query for notification
     * attribute.
     * @param query Query to be parsed for attributes.
     * @param type Type of the Base Object.
     * @return true if required attributes are present else false.
     */
    virtual bool parse_notification_attribute(char *&query,
                                              M2MBase::BaseType type);

protected : // from M2MTimerObserver

    virtual void timer_expired(M2MTimerObserver::Type type =
                               M2MTimerObserver::Notdefined);

private:

    enum {
        Cancel = 1,
        Pmin = 2,
        Pmax = 4,
        Lt = 8,
        Gt = 16,
        St = 32
    };

    bool set_notification_attribute(char* option, M2MBase::BaseType type);

    /**
     * @brief Schedule a report, if the pmin is exceeded
     * then report immediately else store the state to be
     * reported once the time fires.
     */
    void schedule_report(float value);

    /**
    * @brief Reports a sample that satisfies the reporting criteria.
    */
    void report(float value);

    /**
     * @brief Reports that object or object instance satisfies the reporting criteria.
    */
    void report_object_notification();

    /**
    * @brief Manage timers for pmin and pmax.
    */
    void handle_timers();

    /**
    * @brief Check whether notification params can be accepted.
    */
    bool check_attribute_validity();

    /**
    * @brief Stop pmin & pmax timers.
    */
    void stop_timers();

    /**
    * @brief Set back to default values.
    */
    void set_default_values();

    /**
     * @brief Check if current value match threshold values.
     * @return True if notify can be send otherwise false.
     */
    bool check_threshold_values();

    /**
     * @brief Check whether current value matches with GT & LT.
     * @return True if current value match with GT or LT values.
     */
    bool check_gt_lt_params();

private:

    M2MReportObserver           &_observer;
    bool                        _under_observation;    
    float                       _pmax;
    float                       _pmin;
    float                       _gt;
    float                       _lt;
    float                       _st;
    bool                        _pmin_exceeded;
    bool                        _pmax_exceeded;
    bool                        _report_scheduled;    
    M2MTimer                    *_pmin_timer;
    M2MTimer                    *_pmax_timer;        
    float                       _high_step;
    float                       _low_step;
    float                       _current_value;
    float                       _last_value;    
    int                         _attribute_state;    
    bool                        _notify;

friend class Test_M2MReportHandler;

};

#endif // M2MREPORTHANDLER_H
