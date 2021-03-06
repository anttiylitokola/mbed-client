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
#include "mbed-client/m2mobjectinstance.h"
#include "mbed-client/m2mobject.h"
#include "mbed-client/m2mconstants.h"
#include "mbed-client/m2mresource.h"
#include "mbed-client/m2mresource.h"
#include "mbed-client/m2mobservationhandler.h"
#include "include/m2mtlvserializer.h"
#include "include/m2mtlvdeserializer.h"
#include "include/nsdllinker.h"
#include "include/m2mreporthandler.h"
#include "ns_trace.h"

M2MObjectInstance& M2MObjectInstance::operator=(const M2MObjectInstance& other)
{
    if (this != &other) { // protect against invalid self-assignment
        if(!other._resource_list.empty()){
            M2MResource* ins = NULL;
            M2MResourceList::const_iterator it;
            it = other._resource_list.begin();
            for (; it!=other._resource_list.end(); it++ ) {
                ins = *it;
                _resource_list.push_back(new M2MResource(*ins));
            }
        }
    }
    return *this;
}

M2MObjectInstance::M2MObjectInstance(const M2MObjectInstance& other)
: M2MBase(other),
  _object_callback(other._object_callback)
{
    this->operator=(other);
}

M2MObjectInstance::M2MObjectInstance(const String &object_name,
                                     M2MObjectCallback &object_callback)
: M2MBase(object_name,M2MBase::Dynamic),
  _object_callback(object_callback)
{
    M2MBase::set_base_type(M2MBase::ObjectInstance);
    if(M2MBase::name_id() != -1) {
        M2MBase::set_coap_content_type(99);
    }
}

M2MObjectInstance::~M2MObjectInstance()
{
    if(!_resource_list.empty()) {
        M2MResource* res = NULL;
        M2MResourceList::const_iterator it;
        it = _resource_list.begin();
        for (; it!=_resource_list.end(); it++ ) {
            //Free allocated memory for resources.
            res = *it;
            delete res;
            res = NULL;
        }
        _resource_list.clear();
    }
}

M2MResource* M2MObjectInstance::create_static_resource(const String &resource_name,
                                                       const String &resource_type,
                                                       M2MResourceInstance::ResourceType type,
                                                       const uint8_t *value,
                                                       const uint8_t value_length,
                                                       bool multiple_instance)
{
    tr_debug("M2MObjectInstance::create_static_resource(resource_name %s)",resource_name.c_str());
    M2MResource *res = NULL;
    if( resource_name.empty() ){
        return res;
    }
    res = new M2MResource(*this,resource_name, resource_type, type,
                               value, value_length, multiple_instance);
    if(res) {
        _resource_list.push_back(res);
    }
    return res;
}

M2MResource* M2MObjectInstance::create_dynamic_resource(const String &resource_name,
                                                const String &resource_type,
                                                M2MResourceInstance::ResourceType type,
                                                bool observable,
                                                bool multiple_instance)
{
    tr_debug("M2MObjectInstance::create_dynamic_resource(resource_name %s)",resource_name.c_str());
    M2MResource *res = NULL;
    if( resource_name.empty() ){
        return res;
    }
    res = new M2MResource(*this,resource_name, resource_type, type,
                          observable, multiple_instance);
    if(res) {
        _resource_list.push_back(res);
    }
    return res;
}

M2MResourceInstance* M2MObjectInstance::create_static_resource_instance(const String &resource_name,
                                                                        const String &resource_type,
                                                                        M2MResourceInstance::ResourceType type,
                                                                        const uint8_t *value,
                                                                        const uint8_t value_length,
                                                                        uint16_t instance_id)
{
    tr_debug("M2MObjectInstance::create_static_resource_instance(resource_name %s)",resource_name.c_str());
    M2MResourceInstance *instance = NULL;
    if( resource_name.empty() ){
        return instance;
    }
    M2MResource *res = resource(resource_name);
    if(!res) {
        res = new M2MResource(*this,resource_name, resource_type, type,
                              value, value_length, true);
        _resource_list.push_back(res);
    }
    if(res->supports_multiple_instances()&& (res->resource_instance(instance_id) == NULL)) {
        instance = new M2MResourceInstance(resource_name, resource_type, type,
                                           value, value_length,*this);
        if(instance) {
            instance->set_operation(M2MBase::GET_ALLOWED);
            instance->set_observable(false);
            instance->set_instance_id(instance_id);
            res->add_resource_instance(instance);
        }
    }
    return instance;
}

M2MResourceInstance* M2MObjectInstance::create_dynamic_resource_instance(const String &resource_name,
                                                                         const String &resource_type,
                                                                         M2MResourceInstance::ResourceType type,
                                                                         bool observable,
                                                                         uint16_t instance_id)
{
    tr_debug("M2MObjectInstance::create_dynamic_resource_instance(resource_name %s)",resource_name.c_str());
    M2MResourceInstance *instance = NULL;
    if( resource_name.empty() ){
        return instance;
    }
    M2MResource *res = resource(resource_name);
    if(!res) {
        res = new M2MResource(*this,resource_name, resource_type, type,
                          observable, true);
        _resource_list.push_back(res);
    }
    if(res->supports_multiple_instances() && (res->resource_instance(instance_id) == NULL)) {
        instance = new M2MResourceInstance(resource_name, resource_type, type,*this);
        if(instance) {
            instance->set_operation(M2MBase::GET_PUT_ALLOWED);
            instance->set_observable(observable);
            instance->set_instance_id(instance_id);
            res->add_resource_instance(instance);
        }
    }
    return instance;
}

bool M2MObjectInstance::remove_resource(const String &resource_name)
{
    tr_debug("M2MObjectInstance::remove_resource(resource_name %s)",
             resource_name.c_str());
    bool success = false;
    if(!_resource_list.empty()) {
         M2MResource* res = NULL;
         M2MResourceList::const_iterator it;
         it = _resource_list.begin();
         int pos = 0;
         for ( ; it != _resource_list.end(); it++, pos++ ) {
             if(((*it)->name() == resource_name)) {
                // Resource found and deleted.
                res = *it;

                char *obj_inst_id = (char*)malloc(20);
                if(obj_inst_id) {
                    snprintf(obj_inst_id, 20,"%d",instance_id());

                    String obj_name = name();
                    obj_name += String("/");
                    obj_name += String(obj_inst_id);
                    obj_name += String("/");
                    obj_name += (*it)->name();

                    free(obj_inst_id);


                    remove_resource_from_coap(obj_name);
                    delete res;
                    res = NULL;
                    _resource_list.erase(pos);
                    success = true;
                }
                 break;
             }
         }
     }
    return success;
}

bool M2MObjectInstance::remove_resource_instance(const String &resource_name,
                                                 uint16_t inst_id)
{
    tr_debug("M2MObjectInstance::remove_resource_instance(resource_name %s inst_id %d)",
             resource_name.c_str(), inst_id);
    bool success = false;
    M2MResource *res = resource(resource_name);
    if(res) {
        M2MResourceInstanceList list = res->resource_instances();
        M2MResourceInstanceList::const_iterator it;
        it = list.begin();
        for ( ; it != list.end(); it++) {
            if((*it)->instance_id() == inst_id) {
                char *obj_inst_id = (char*)malloc(20);
                if(obj_inst_id) {
                    snprintf(obj_inst_id, 20,"%d",instance_id());

                    String obj_name = name();
                    obj_name += String("/");
                    obj_name += String(obj_inst_id);
                    obj_name += String("/");
                    obj_name += resource_name;

                    free(obj_inst_id);

                    char *res_inst_id = (char*)malloc(20);
                    if(res_inst_id) {
                        snprintf(res_inst_id, 20,"%d",inst_id);
                        obj_name += String("/");
                        obj_name += String(res_inst_id);

                        free(res_inst_id);

                        remove_resource_from_coap(obj_name);
                        success = res->remove_resource_instance(inst_id);
                        if(res->resource_instance_count() == 0) {
                            M2MResourceList::const_iterator itr;
                            itr = _resource_list.begin();
                            int pos = 0;
                            for ( ; itr != _resource_list.end(); itr++, pos++ ) {
                                if(((*itr)->name() == resource_name)) {
                                    delete res;
                                    res = NULL;
                                    _resource_list.erase(pos);
                                    break;
                                }
                            }
                        }
                    }
                }
                break;
            }
        }
    }
    return success;
}

M2MResource* M2MObjectInstance::resource(const String &resource) const
{
    M2MResource *res = NULL;
    if(!_resource_list.empty()) {
        M2MResourceList::const_iterator it;
        it = _resource_list.begin();
        for (; it!=_resource_list.end(); it++ ) {
            if((*it)->name() == resource) {
                res = *it;
                break;
            }
        }
    }
    return res;
}

const M2MResourceList& M2MObjectInstance::resources() const
{
    return _resource_list;
}

uint16_t M2MObjectInstance::resource_count() const
{
    uint16_t count = 0;
    if(!_resource_list.empty()) {
        M2MResourceList::const_iterator it;
        it = _resource_list.begin();
        for ( ; it != _resource_list.end(); it++ ) {
            if((*it)->supports_multiple_instances()) {
                count += (*it)->resource_instance_count();
            } else {
                count++;
            }
        }
    }
    return count;
}

uint16_t M2MObjectInstance::resource_count(const String& resource) const
{
    uint16_t count = 0;
    if(!_resource_list.empty()) {
        M2MResourceList::const_iterator it;
        it = _resource_list.begin();
        for ( ; it != _resource_list.end(); it++ ) {
            if((*it)->name() == resource) {
                if((*it)->supports_multiple_instances()) {
                    count += (*it)->resource_instance_count();
                } else {
                    count++;
                }
            }
        }
    }
    return count;
}

M2MBase::BaseType M2MObjectInstance::base_type() const
{
    return M2MBase::base_type();
}

void M2MObjectInstance::add_observation_level(M2MBase::Observation observation_level)
{
    M2MBase::add_observation_level(observation_level);
    if(!_resource_list.empty()) {
        M2MResourceList::const_iterator it;
        it = _resource_list.begin();
        for ( ; it != _resource_list.end(); it++ ) {            
            (*it)->add_observation_level(observation_level);
        }
    }
}

void M2MObjectInstance::remove_observation_level(M2MBase::Observation observation_level)
{
    M2MBase::remove_observation_level(observation_level);
    if(!_resource_list.empty()) {
        M2MResourceList::const_iterator it;
        it = _resource_list.begin();
        for ( ; it != _resource_list.end(); it++ ) {            
            (*it)->remove_observation_level(observation_level);
        }
    }
}

sn_coap_hdr_s* M2MObjectInstance::handle_get_request(nsdl_s *nsdl,
                                                     sn_coap_hdr_s *received_coap_header,
                                                     M2MObservationHandler *observation_handler)
{
    tr_debug("M2MObjectInstance::handle_get_request()");
    sn_coap_hdr_s * coap_response = NULL;
    uint8_t * data = NULL;
    uint32_t  data_length = 0;
    //TODO: GET for Object is not yet implemented.
    // Need to first fix C library and then implement on C++ side.
    if(received_coap_header) {
        // process the GET if we have registered a callback for it
        if ((operation() & SN_GRS_GET_ALLOWED) != 0) {
            coap_response = sn_nsdl_build_response(nsdl,
                                                   received_coap_header,
                                                   COAP_MSG_CODE_RESPONSE_CONTENT);
            if(coap_response) {
                if(received_coap_header->content_type_ptr){
                    coap_response->content_type_ptr = (uint8_t*)malloc(received_coap_header->content_type_len);
                    if(coap_response->content_type_ptr) {
                        memset(coap_response->content_type_ptr, 0, received_coap_header->content_type_len);
                        memcpy(coap_response->content_type_ptr,
                               received_coap_header->content_type_ptr,
                               received_coap_header->content_type_len);
                        coap_response->content_type_len = received_coap_header->content_type_len;
                     }
                } else {
                    uint8_t content_type = M2MBase::coap_content_type();

                    coap_response->content_type_ptr = (uint8_t*)malloc(1);
                    if(coap_response->content_type_ptr) {
                        memset(coap_response->content_type_ptr, 0, 1);
                        memcpy(coap_response->content_type_ptr,&content_type,1);
                        coap_response->content_type_len = 1;
                    }
                }

                // fill in the CoAP response payload
                if(COAP_CONTENT_OMA_TLV_TYPE == *coap_response->content_type_ptr &&
                   COAP_CONTENT_OMA_TLV_TYPE == M2MBase::coap_content_type()) {
                    M2MTLVSerializer *serializer = new M2MTLVSerializer();
                    if(serializer) {
                        data = serializer->serialize(_resource_list, data_length);
                        delete serializer;
                    }
                } else if(*coap_response->content_type_ptr == COAP_CONTENT_OMA_JSON_TYPE) {
                    // TOD0: Implement JSON Format.
                    coap_response->msg_code = COAP_MSG_CODE_RESPONSE_UNSUPPORTED_CONTENT_FORMAT; // Content format not supported
                } else {
                    coap_response->msg_code = COAP_MSG_CODE_RESPONSE_UNSUPPORTED_CONTENT_FORMAT; // Content format not supported
                }

                coap_response->payload_len = data_length;
                coap_response->payload_ptr = data;

                if(data) {
                    coap_response->options_list_ptr = (sn_coap_options_list_s*)malloc(sizeof(sn_coap_options_list_s));
                    memset(coap_response->options_list_ptr, 0, sizeof(sn_coap_options_list_s));

                    coap_response->options_list_ptr->max_age_ptr = (uint8_t*)malloc(1);
                    memset(coap_response->options_list_ptr->max_age_ptr,0,1);
                    coap_response->options_list_ptr->max_age_len = 1;


                    if(received_coap_header->token_ptr) {
                        tr_debug("M2MObjectInstance::handle_get_request - Sets Observation Token to resource");
                        set_observation_token(received_coap_header->token_ptr,
                                              received_coap_header->token_len);
                    }
                    if(received_coap_header->options_list_ptr) {
                        if(received_coap_header->options_list_ptr->observe) {
                            uint32_t number = 0;
                            uint8_t observe_option = 0;
                            if(received_coap_header->options_list_ptr->observe_ptr) {
                                observe_option = *received_coap_header->options_list_ptr->observe_ptr;
                            }
                            if(START_OBSERVATION == observe_option) {
                                tr_debug("M2MObjectInstance::handle_get_request - Starts Observation");
                                // If the observe length is 0 means register for observation.
                                if(received_coap_header->options_list_ptr->observe_len != 0) {
                                    for(int i=0;i < received_coap_header->options_list_ptr->observe_len; i++) {
                                        number = (*(received_coap_header->options_list_ptr->observe_ptr + i) & 0xff) <<
                                                 8*(received_coap_header->options_list_ptr->observe_len- 1 - i);
                                    }
                                }
                                // If the observe value is 0 means register for observation.
                                if(number == 0) {
                                    tr_debug("M2MResource::handle_get_request - Put Resource under Observation");
                                    set_under_observation(true,observation_handler);
                                    add_observation_level(M2MBase::OI_Attribute);

                                    uint8_t *obs_number = (uint8_t*)malloc(3);
                                    memset(obs_number,0,3);
                                    uint8_t observation_number_length = 1;

                                    uint16_t number = observation_number();

                                    tr_debug("M2MResource::handle_get_request - Observation Number %d", number);
                                    obs_number[0] = ((number>>8) & 0xFF);
                                    obs_number[1] = (number & 0xFF);

                                    if(number > 0xFF) {
                                        observation_number_length = 2;
                                    }
                                    coap_response->options_list_ptr->observe_ptr = obs_number;
                                    coap_response->options_list_ptr->observe_len = observation_number_length;
                                }
                            } else if (STOP_OBSERVATION == observe_option) {
                                tr_debug("M2MResource::handle_get_request - Stops Observation");
                                set_under_observation(false,NULL);
                                remove_observation_level(M2MBase::OI_Attribute);

                            }
                        }
                    }
                } else {
                    coap_response->msg_code = COAP_MSG_CODE_RESPONSE_UNSUPPORTED_CONTENT_FORMAT; // Content format not supported
                }
            }
        }else {
            tr_error("M2MResource::handle_get_request - Return COAP_MSG_CODE_RESPONSE_BAD_REQUEST");
            // Operation is not allowed.
            coap_response = sn_nsdl_build_response(nsdl,
                                                   received_coap_header,
                                                   COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
            if(coap_response) {
                coap_response->options_list_ptr = 0;
                coap_response->content_type_ptr = 0;
            }
        }
    }
    return coap_response;
}

sn_coap_hdr_s* M2MObjectInstance::handle_put_request(nsdl_s *nsdl,
                                                     sn_coap_hdr_s *received_coap_header,
                                                     M2MObservationHandler *observation_handler)
{
    tr_debug("M2MObjectInstance::handle_put_request()");
    sn_coap_hdr_s * coap_response = NULL;
    //TODO: POST for Object is not yet implemented.
    // Need to first fix C library and then implement on C++ side.
    if(received_coap_header) {
        if ((operation() & SN_GRS_PUT_ALLOWED) != 0) {
            sn_coap_msg_code_e msg_code = COAP_MSG_CODE_RESPONSE_CHANGED; // 2.04
            if(received_coap_header->content_type_ptr) {
                if(*received_coap_header->content_type_ptr == COAP_CONTENT_OMA_TLV_TYPE) {
                    M2MTLVDeserializer *deserializer = new M2MTLVDeserializer();
                    if(deserializer) {
                        if(deserializer->is_object_instance(received_coap_header->payload_ptr)) {
                            deserializer->deserialize_resources(received_coap_header->payload_ptr,
                                                                received_coap_header->payload_len,
                                                                _resource_list);
                        } else {
                            msg_code = COAP_MSG_CODE_RESPONSE_BAD_REQUEST; // 4.00
                        }
                        delete deserializer;
                    }
                }
            }
            if(received_coap_header->options_list_ptr &&
               received_coap_header->options_list_ptr->uri_query_ptr) {
                char *query = (char*)malloc(received_coap_header->options_list_ptr->uri_query_len+1);
                if (query){
                    memset(query, 0, received_coap_header->options_list_ptr->uri_query_len+1);
                    memcpy(query,
                        received_coap_header->options_list_ptr->uri_query_ptr,
                        received_coap_header->options_list_ptr->uri_query_len);
                    memset(query + received_coap_header->options_list_ptr->uri_query_len,'\0',1);//String terminator
                   tr_debug("M2MObjectInstance::handle_put_request() - Query %s", query);
                    // if anything was updated, re-initialize the stored notification attributes
                    if (!handle_observation_attribute(query)){
                        tr_debug("M2MResourceInstance::handle_put_request() - Invalid query");
                        msg_code = COAP_MSG_CODE_RESPONSE_BAD_REQUEST; // 4.00
                    }
                    free(query);
                }
                if(observation_handler) {
                    observation_handler->value_updated(this);
                }
            }
            coap_response = sn_nsdl_build_response(nsdl,
                                                   received_coap_header,
                                                   msg_code);
        } else {
            // Operation is not allowed.
            tr_error("M2MObjectInstance::handle_put_request() - COAP_MSG_CODE_RESPONSE_BAD_REQUEST");
            coap_response = sn_nsdl_build_response(nsdl,
                                                   received_coap_header,
                                                   COAP_MSG_CODE_RESPONSE_BAD_REQUEST);
            if(coap_response) {
                coap_response->options_list_ptr = 0;
                coap_response->content_type_ptr = 0;
            }
        }
    }
    return coap_response;
}

sn_coap_hdr_s* M2MObjectInstance::handle_post_request(nsdl_s *nsdl,
                                                      sn_coap_hdr_s *received_coap_header,
                                                      M2MObservationHandler *observation_handler)
{
    tr_debug("M2MObjectInstance::handle_post_request()");
    sn_coap_hdr_s * coap_response = NULL;
    //TODO: POST for Object is not yet implemented.
    // Need to first fix C library and then implement on C++ side.
    sn_coap_msg_code_e msg_code = COAP_MSG_CODE_RESPONSE_NOT_IMPLEMENTED; // 5.01
    coap_response = sn_nsdl_build_response(nsdl,
                                           received_coap_header,
                                           msg_code);
    return coap_response;
}

void M2MObjectInstance::notification_update(M2MBase::Observation observation_level)
{
    if(M2MBase::O_Attribute == observation_level) {
        _object_callback.notification_update();
    } else {
        M2MReportHandler *report_handler = M2MBase::report_handler();
        if(report_handler) {
            report_handler->trigger_object_notification();
        }
    }
}
