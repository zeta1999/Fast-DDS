// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef HEADER_REDUCTION_TRANSPORT_H
#define HEADER_REDUCTION_TRANSPORT_H

#include "../ChainingTransport.h"
#include "HeaderReductionTransportDescriptor.h"

#include <mutex>
#include <stdio.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct HeaderReductionOptions;

/**
 * An adapter transport for bandwidth reduction.
 * This transport performs a specific compression of data before sending and the
 * corresponding decompression after receiving. The compression algorithm is specific
 * for the RTPS protocol. It will remove certain headers while compressing others.
 * This transport is configured using the following participant properties:
 * - rtps.header_reduction.remove_protocol: true or false
 * - rtps.header_reduction.remove_version: true or false
 * - rtps.header_reduction.remove_vendor_id: true or false
 * - rtps.header_reduction.compress_guid_prefix: a, b, c (3 numbers from 8 to 32)
 * - rtps.header_reduction.submessage.combine_id_and_flags: true or false
 * - rtps.header_reduction.submessage.remove_extra_flags: true or false
 * - rtps.header_reduction.submessage.compress_entitiy_ids: r, w (number from 8 to 32)
 * - rtps.header_reduction.submessage.compress_sequence_number: n (number from 16 to 64)
 * @ingroup TRANSPORT_MODULE
 */
class HeaderReductionTransport : public ChainingTransport
{

public:

    RTPS_DllAPI HeaderReductionTransport(
            const HeaderReductionTransportDescriptor&);

    virtual ~HeaderReductionTransport();

    virtual bool init(
            const fastrtps::rtps::PropertyPolicy* properties = nullptr) override;

    TransportDescriptorInterface* get_configuration() override
    {
        return &configuration_;
    }

    /**
     * Blocking Send through the specified channel. It will perform compression and then send the compressed
     * data to the lower transport.
     * @param low_sender_resource SenderResource generated by the lower transport.
     * @param send_buffer Slice into the raw data to send.
     * @param send_buffer_size Size of the raw data. It will be used as a bounds check for the previous argument.
     * It must not exceed the sendBufferSize fed to this class during construction.
     * @param remote_locator Locator describing the remote destination we're sending to.
     * @param timeout Maximum blocking time.
     */
    bool send(
            fastrtps::rtps::SenderResource* low_sender_resource,
            const fastrtps::rtps::octet* send_buffer,
            uint32_t send_buffer_size,
            fastrtps::rtps::LocatorsIterator* destination_locators_begin,
            fastrtps::rtps::LocatorsIterator* destination_locators_end,
            const std::chrono::steady_clock::time_point& timeout) override;

    /**
     * Blocking Receive from the specified channel. It will receive from the lower transport and then perform
     * decompression of data.
     * @param next_receiver Next resource receiver to be called.
     * @param receive_buffer vector with enough capacity (not size) to accomodate a full receive buffer. That
     * capacity must not be less than the receiveBufferSize supplied to this class during construction.
     * @param local_locator Locator mapping to the local channel we're listening to.
     * @param[out] remote_locator Locator describing the remote restination we received a packet from.
     */
    void receive(
            TransportReceiverInterface* next_receiver,
            const fastrtps::rtps::octet* receive_buffer,
            uint32_t receive_buffer_size,
            const fastrtps::rtps::Locator_t& local_locator,
            const fastrtps::rtps::Locator_t& remote_locator) override;

    uint32_t max_recv_buffer_size() const override
    {
        return low_level_transport_->max_recv_buffer_size();
    }

protected:

    //! Size of the underlying transport buffer.
    uint32_t buffer_size_;
    //! Transport options. Filled when calling to init.
    std::unique_ptr<HeaderReductionOptions> options_;
    //! Only one thread should have access to the compression buffer
    mutable std::mutex compress_buffer_mutex_;
    //! Compression buffer
    fastrtps::rtps::octet* compress_buffer_;
    //! Transport configuration.
    HeaderReductionTransportDescriptor configuration_;

#if HEAD_REDUCTION_DEBUG_DUMP
    //! Debug dump files
    FILE* dump_file_;
    FILE* dump_file_low_;
#endif
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif
