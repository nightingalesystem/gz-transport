/*
 * Copyright (C) 2014 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef __IGN_TRANSPORT_DISCOVERY_HH_INCLUDED__
#define __IGN_TRANSPORT_DISCOVERY_HH_INCLUDED__

#ifdef _MSC_VER
# pragma warning(push, 0)
#endif

#ifdef _WIN32
  // For socket(), connect(), send(), and recv().
  #include <Winsock2.h>
  // Type used for raw data on this platform.
  using raw_type = char;
#else
  // For sockaddr_in
  #include <netinet/in.h>
  // Type used for raw data on this platform
  using raw_type = void;
#endif

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#ifdef _MSC_VER
# pragma warning(pop)
#endif

#include "ignition/transport/Helpers.hh"
#include "ignition/transport/Packet.hh"
#include "ignition/transport/TransportTypes.hh"

namespace ignition
{
  namespace transport
  {
    class DiscoveryPrivate;
    class Publisher;

    /// \class Discovery Discovery.hh ignition/transport/Discovery.hh
    /// \brief A discovery class that implements a distributed topic discovery
    /// protocol. It uses UDP broadcast for sending/receiving messages and
    /// stores updated topic information. The discovery clients can request
    /// the discovery of a topic or the advertisement of a local topic. The
    /// discovery uses heartbeats to track the state of other peers in the
    /// network. The discovery clients can register callbacks to detect when
    /// new topics are discovered or topics are no longer available.
    class IGNITION_VISIBLE Discovery
    {
      /// \brief Constructor.
      /// \param[in] _pUuid This discovery instance will run inside a
      /// transport process. This parameter is the transport process' UUID.
      /// \param[in] _verbose true for enabling verbose mode.
      public: Discovery(const std::string &_pUuid, bool _verbose = false);

      /// \brief Destructor.
      public: virtual ~Discovery();

      /// \brief Start the discovery service. You probably want to register the
      /// callbacks for receiving discovery notifications before starting the
      /// service.
      public: void Start();

      /// \brief Advertise a new message/service.
      /// \param[in] _publisher Publisher's information to advertise.
      /// \return True if the method succeed or false otherwise
      /// (e.g. if the discovery has not been started).
      public: bool Advertise(const Publisher& _publisher);

      /// \brief Request discovery information about a topic/service.
      /// When using this method, the user might want to use
      /// ConnectionsCb() and DisconnectionCb()
      /// or ConnectionsSrvCb() and DisconnectionSrvCb(),
      /// that registers callbacks
      /// that will be executed when the topic/service address is discovered
      /// or when the node providing the topic/service is disconnected.
      /// \sa SetConnectionsCb.
      /// \sa SetDisconnectionsCb.
      /// \sa SetConnectionsSrvCb.
      /// \sa SetDisconnectionsSrvCb.
      /// \param[in] _topic Topic name requested.
      /// \return True if the method succeeded or false otherwise
      /// (e.g. if the discovery has not been started).
      public: bool Discover(const std::string &_topic);

      /// \brief Get all the publishers' information known for a given topic.
      /// \param[in] _topic Topic name.
      /// \param[out] _publishers Publishers requested.
      /// \return True if the topic is found and there is at least one publisher
      public: bool MsgPublishers(const std::string &_topic,
                                 Addresses_M &_publishers);

      /// \brief Get all the publishers' information known for a given service.
      /// \param[in] _topic Service name.
      /// \param[out] _publishers Publishers requested.
      /// \return True if the topic is found and there is at least one publisher
      public: bool SrvPublishers(const std::string &_topic,
                                 Addresses_M &_publishers);

      /// \brief Unadvertise a new message/service. Broadcast a discovery
      /// message that will cancel all the discovery information for
      /// the topic/service advertised by a specific node.
      /// \param[in] _topic Topic/Service name to be unadvertised.
      /// \param[in] _nUuid Node UUID.
      /// \return True if the method succeeded or false otherwise
      /// (e.g. if the discovery has not been started).
      public: bool Unadvertise(const std::string &_topic,
                                  const std::string &_nUuid);

      /// \brief Get the IP address of this host.
      /// \return A string with this host's IP address.
      public: std::string HostAddr() const;

      /// \brief The discovery checks the validity of the topic information
      /// every 'activity interval' milliseconds.
      /// \sa SetActivityInterval.
      /// \return The value in milliseconds.
      public: unsigned int ActivityInterval() const;

      /// \brief Each node broadcasts periodic heartbeats to keep its topic
      /// information alive in other nodes. A heartbeat message is sent after
      /// 'heartbeat interval' milliseconds.
      /// \sa SetHeartbeatInterval.
      /// \return The value in milliseconds.
      public: unsigned int HeartbeatInterval() const;

      /// \brief While a topic is being advertised by a node, a beacon is sent
      /// periodically every 'advertise interval' milliseconds.
      /// \sa SetAdvertiseInterval.
      /// \return The value in milliseconds.
      public: unsigned int AdvertiseInterval() const;

      /// \brief Get the maximum time allowed without receiving any discovery
      /// information from a node before canceling its entries.
      /// \sa SetSilenceInterval.
      /// \return The value in milliseconds.
      public: unsigned int SilenceInterval() const;

      /// \brief Set the activity interval.
      /// \sa GetActivityInterval.
      /// \param[in] _ms New value in milliseconds.
      public: void ActivityInterval(const unsigned int _ms);

      /// \brief Set the heartbeat interval.
      /// \sa GetHeartbeatInterval.
      /// \param[in] _ms New value in milliseconds.
      public: void HeartbeatInterval(const unsigned int _ms);

      /// \brief Set the advertise interval.
      /// \sa GetAdvertiseInterval.
      /// \param[in] _ms New value in milliseconds.
      public: void AdvertiseInterval(const unsigned int _ms);

      /// \brief Set the maximum silence interval.
      /// \sa GetSilenceInterval.
      /// \param[in] _ms New value in milliseconds.
      public: void SilenceInterval(const unsigned int _ms);

      /// \brief Register a callback to receive discovery connection events.
      /// Each time a new topic is connected, the callback will be executed.
      /// This version uses a free function as callback.
      /// \param[in] _cb Function callback.
      public: void ConnectionsCb(const DiscoveryCallback &_cb);

      /// \brief Register a callback to receive discovery connection events.
      /// Each time a new topic is discovered, the callback will be executed.
      /// This version uses a member functions as callback.
      /// \param[in] _cb Function callback with the following parameters.
      ///                _pub Publisher's information.
      /// \param[in] _obj Object instance where the member function belongs.
      public: template<typename C> void ConnectionsCb(
        void(C::*_cb)(const Publisher &_pub),
        C *_obj)
      {
        this->ConnectionsCb(std::bind(_cb, _obj, std::placeholders::_1));
      }

      /// \brief Register a callback to receive discovery disconnection events.
      /// Each time a topic is no longer active, the callback will be executed.
      /// This version uses a free function as callback.
      /// \param[in] _cb Function callback.
      public: void DisconnectionsCb(const transport::DiscoveryCallback &_cb);

      /// \brief Register a callback to receive discovery disconnection events.
      /// Each time a topic is no longer active, the callback will be executed.
      /// This version uses a member function as callback.
      /// \param[in] _cb Function callback with the following parameters.
      ///                _pub Publisher's information.
      /// \param[in] _obj Object instance where the member function belongs.
      public: template<typename C> void DisconnectionsCb(
        void(C::*_cb)(const Publisher &_pub),
        C *_obj)
      {
        this->DisconnectionsCb(std::bind(_cb, _obj, std::placeholders::_1));
      }

      /// \brief Register a callback to receive discovery connection events for
      /// services.
      /// Each time a new service is available, the callback will be executed.
      /// This version uses a free function as callback.
      /// \param[in] _cb Function callback.
      public: void ConnectionsSrvCb(const DiscoveryCallback &_cb);

      /// \brief Register a callback to receive discovery connection events for
      /// services.
      /// Each time a new service is available, the callback will be executed.
      /// This version uses a member functions as callback.
      /// \param[in] _cb Function callback with the following parameters.
      ///                _pub Publisher's information.
      /// \param[in] _obj Object instance where the member function belongs.
      public: template<typename C> void ConnectionsSrvCb(
        void(C::*_cb)(const Publisher &_pub),
        C *_obj)
      {
        this->ConnectionsSrvCb(std::bind(_cb, _obj, std::placeholders::_1));
      }

      /// \brief Register a callback to receive discovery disconnection events
      /// for services.
      /// Each time a service is no longer available, the callback will be
      /// executed.
      /// This version uses a free function as callback.
      /// \param[in] _cb Function callback.
      public: void DisconnectionsSrvCb(
        const transport::DiscoveryCallback &_cb);

      /// \brief Register a callback to receive discovery disconnection events.
      /// Each time a service is no longer available, the callback will be
      /// executed.
      /// This version uses a member function as callback.
      /// \param[in] _cb Function callback with the following parameters.
      ///                _pub Publisher's information.
      /// \param[in] _obj Object instance where the member function belongs.
      public: template<typename C> void DisconnectionsSrvCb(
        void(C::*_cb)(const Publisher &_pub), C *_obj)
      {
        this->DisconnectionsSrvCb(std::bind(_cb, _obj, std::placeholders::_1));
      }

      /// \brief Print the current discovery state (info, activity, unknown).
      public: void PrintCurrentState();

      /// \brief Get the list of topics currently advertised in the network.
      /// \param[out] _topics List of advertised topics.
      public: void TopicList(std::vector<std::string> &_topics) const;

      /// \brief Get the list of services currently advertised in the network.
      /// \param[out] _topics List of advertised services.
      public: void ServiceList(std::vector<std::string> &_services) const;

      /// \brief Get mutex used in the Discovery class.
      /// \return The discovery mutex.
      public: std::recursive_mutex& Mutex();

      /// \brief Check the validity of the topic information. Each topic update
      /// has its own timestamp. This method iterates over the list of topics
      /// and invalids the old topics.
      private: void RunActivityTask();

      /// \brief Broadcast periodic heartbeats.
      private: void RunHeartbeatTask();

      /// \brief Receive discovery messages.
      private: void RunReceptionTask();

      /// \brief Method in charge of receiving the discovery updates.
      private: void RecvDiscoveryUpdate();

      /// \brief Parse a discovery message received via the UDP broadcast socket
      /// \param[in] _fromIp IP address of the message sender.
      /// \param[in] _msg Received message.
      private: void DispatchDiscoveryMsg(const std::string &_fromIp,
                                         char *_msg);

      /// \brief Broadcast a discovery message.
      /// \param[in] _type Message type.
      /// \param[in] _pub Publishers's information to send.
      /// \param[in] _flags Optional flags. Currently, the flags are not used
      /// but they will in the future for specifying things like compression,
      /// or encryption.
      private: void SendMsg(uint8_t _type, const Publisher &_pub,
                            int _flags = 0);

      /// \brief Get the list of sockets used for discovery.
      /// \return The list of sockets.
      private: std::vector<int>& Sockets() const;

      /// \brief Get the data structure used for multicast communication.
      /// \return The data structure containing the multicast information.
      private: sockaddr_in* MulticastAddr() const;

      /// \brief Get the verbose mode.
      /// \return True when verbose mode is enabled or false otherwise.
      private: bool Verbose() const;

      /// \brief Get the discovery protocol version.
      /// \return The discovery version.
      private: uint8_t Version() const;

      /// \internal
      /// \brief Smart pointer to private data.
      protected: std::unique_ptr<DiscoveryPrivate> dataPtr;
    };
  }
}
#endif
