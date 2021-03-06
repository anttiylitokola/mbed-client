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
#ifndef M2MREPORTOBSERVER_H
#define M2MREPORTOBSERVER_H

//FORWARD DECLARATION
class M2MResourceInstance;

/**
 * @brief M2MReportObserver
 * An interface to report to the base class
 * when to send the report to the server.
 *
 */
class M2MReportObserver
{
  public:

    /**
     * @brief Observation callback to be sent to the
     * server because some of the observed parameter has
     * changed.
     */
    virtual void observation_to_be_sent() = 0;

};

#endif // M2MREPORTOBSERVER_H
