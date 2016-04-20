#ifndef ROUTER_HPP_UKYYCAR0
#define ROUTER_HPP_UKYYCAR0

#include <vanetza/common/byte_order.hpp>
#include <vanetza/common/hook.hpp>
#include <vanetza/geonet/beacon_header.hpp>
#include <vanetza/geonet/cbf_packet_buffer.hpp>
#include <vanetza/geonet/common_header.hpp>
#include <vanetza/geonet/extended_pdu.hpp>
#include <vanetza/geonet/gbc_header.hpp>
#include <vanetza/geonet/interface.hpp>
#include <vanetza/geonet/location_table.hpp>
#include <vanetza/geonet/mib.hpp>
#include <vanetza/geonet/packet.hpp>
#include <vanetza/geonet/packet_buffer.hpp>
#include <vanetza/geonet/pdu.hpp>
#include <vanetza/geonet/pdu_variant.hpp>
#include <vanetza/geonet/repeater.hpp>
#include <vanetza/geonet/sequence_number.hpp>
#include <vanetza/geonet/shb_header.hpp>
#include <vanetza/geonet/timestamp.hpp>
#include <vanetza/units/length.hpp>
#include <vanetza/units/time.hpp>
#include <vanetza/security/security_entity.hpp>
#include <boost/variant.hpp>
#include <cstdint>
#include <memory>
#include <random>
#include <map>

namespace vanetza
{

// forward declarations
class MacAddress;
namespace dcc
{
    struct DataRequest;
    class RequestInterface;
} // namespace dcc

namespace geonet
{

extern const uint16be_t ether_type;

class IndicationContext;
class TransportInterface;
class NextHop;
struct ShbDataRequest;
struct GbcDataRequest;
struct DataConfirm;
struct DataIndication;

/**
 * Router is the central entity for GeoNet communication
 *
 * Incoming and outgoing GeoNet packets are handled by the router.
 * It may even dispatch own packets (beacons) if necessary.
 */
class Router
{
public:
    typedef std::unique_ptr<DataRequest> DataRequestPtr;
    typedef std::unique_ptr<Pdu> PduPtr;
    typedef std::unique_ptr<DownPacket> DownPacketPtr;
    typedef std::unique_ptr<UpPacket> UpPacketPtr;

    /// Reason for packet drop used by drop hook
    enum class PacketDropReason
    {
        PARSE_BASIC_HEADER,
        PARSE_COMMON_HEADER,
        PARSE_SECURED_HEADER,
        PARSE_EXTENDED_HEADER,
        ITS_PROTOCOL_VERSION,
        DECAP_UNSUCCESSFUL_NON_STRICT,
        DECAP_UNSUCCESSFUL_STRICT,
        HOP_LIMIT,
        PAYLOAD_SIZE,
    };

    Router(const MIB&, dcc::RequestInterface&);
    ~Router();

    /**
     * \brief Request to send payload per single hop broadcast (SHB).
     * If security is enabled, the message gets encapsulated in a security envelope.
     * Returns whether data was valid to be sent.
     *
     * \param request
     * \param payload from upper layers
     * \return result code if packet has been accepted
     */
    DataConfirm request(const ShbDataRequest&, DownPacketPtr);

    /**
     * \brief Request to send payload per GeoBroadcast (GBC).
     * If security is enabled, the message gets encapsulated in a security envelope.
     * Returns whether data was valid to be sent.
     *
     * \param request
     * \param payload from upper layers
     * \return result code if packet has been accepted
     */
    DataConfirm request(const GbcDataRequest&, DownPacketPtr);

    // These three requests are not supported yet
    DataConfirm request(const GucDataRequest&, DownPacketPtr);
    DataConfirm request(const GacDataRequest&, DownPacketPtr);
    DataConfirm request(const TsbDataRequest&, DownPacketPtr);

    /**
     * \brief Handle the received packet on network layer.
     * Packet handling involves these steps:
     * - header processing
     * - packet forwarding
     * - passing to transport layer
     * - security decapsulation
     *
     * \param packet received packet from access layer
     * \param sender MAC address of sender
     * \param destination MAC address of destination (might be broadcast)
     */
    void indicate(UpPacketPtr, const MacAddress& sender, const MacAddress& destination);

    /**
     * \brief When a packet is dropped, this Hook is invoked
     * \tparam PacketDropReason why Router decided to drop packet
     */
    Hook<PacketDropReason> packet_dropped;

    /**
     * \brief Get duration until next required update call
     * \note Duration has an upper bound derived from MIB's itsGnMinimumUpdateFrequencyLPV
     *
     * \return duration until next update
     */
    Clock::duration next_update() const;

    /**
     * \brief Update router time stamp by given duration
     *
     * \param duration time passed since last update
     */
    void update(Clock::duration);

    /**
     * \brief Update router's local position vector
     * \note GN Address of given LongPositionVector is ignored!
     *
     * \param lpv Set positional data according to this argument
     */
    void update(const LongPositionVector&);

    /**
     * \brief Register a transport protocol handler.
     *
     * \param proto
     * \param ifc pass packet to transport layer via this interface
     */
    void set_transport_handler(UpperProtocol, TransportInterface&);

    /**
     * \brief Set clocks within router to the given time
     * This method should only be used at initialisation.
     * Use Router's update methods during normal operation.
     *
     * \param init current time
     */
    void set_time(const Clock::time_point&);

    /**
     * \brief Set Router's own GeoNetworking address
     *
     * \param addr
     */
    void set_address(const Address&);

    /**
     * \brief Get the Contention-Based-Forwarding buffer
     *
     * \return read-only reference to CBF packet buffer
     */
    const CbfPacketBuffer& get_cbf_buffer() const { return m_cbf_buffer; }

    /**
     * \brief Get the LocationTable.
     * The table holds information about neighbouring ITS-Routers.
     *
     * \return read-only reference to LocationTable
     */
    const LocationTable& get_location_table() const { return m_location_table; }

    /**
     * \brief Get the local position vector.
     * This vector describes the current position of the router.
     *
     * \return read-only reference to LongPositionVector
     */
    const LongPositionVector& get_local_position_vector() const { return m_local_position_vector; }

    /**
     * \brief Check if router is outside the sectorial contention area
     * See TS 102 636-4-1 v1.2.3 section E.4 and figure E.2 for details.
     *
     * \param sender
     * \param forwarder
     * \return bool true if either sender or forwarder is outside
     */
    bool outside_sectorial_contention_area(const MacAddress& sender, const MacAddress& forwarder) const;

    /**
     * \brief Set seed for internal random number generator (RNG)
     * RNG is used e.g. for random BEACON jitter
     *
     * \param seed reset RNG's state to this seed
     */
    void set_random_seed(std::uint_fast32_t seed);

    /**
     * \brief Create an initialized Single-Hop-Broadcast PDU
     *
     * \param request
     * \return PDU object
     */
    std::unique_ptr<ShbPdu> create_shb_pdu(const ShbDataRequest&);

    /**
     * \brief Create an initialzed BEACON PDU
     *
     * \return PDU object
     */
    std::unique_ptr<BeaconPdu> create_beacon_pdu();

    /**
     * \brief Create an initialized GeoBroadcast PDU
     *
     * \param request
     * \return PDU object
     */
    std::unique_ptr<GbcPdu> create_gbc_pdu(const GbcDataRequest&);

private:
    typedef std::map<UpperProtocol, TransportInterface*> transport_map_t;

    /**
     * \brief Send BEACON packet to all neighbours with updated position vector.
     * Only to be called when the beacon timer expires.
     */
    void on_beacon_timer_expired();

    /**
     * \brief Reschedule timer for next BEACON transmission
     */
    void reset_beacon_timer();

    /**
     * \brief Process BasicHeader at packet indication.
     * \param ctx Context holding data for further parsing
     */
    void indicate_basic(IndicationContext&);

    /**
     * \brief Process CommonHeader at packet indication.
     * \param ctx Context holding data for further parsing
     * \param basic Previously decoded BasicHeader
     */
    void indicate_common(IndicationContext&, const BasicHeader&);

    /**
     * \brief Process ExtendedHeader at packet indication.
     * \param ctx Context holding data for further parsing
     * \param common Previously decoded CommonHeader
     */
    void indicate_extended(IndicationContext&, const CommonHeader&);

    /**
     * \brief Process SecuredMessage at packet indication.
     * \param ctx Context holding data for further parsing
     * \param basic Previously decoded BasicHeader
     */
    void indicate_secured(IndicationContext&, const BasicHeader&);

    /**
     * \brief Process ExtendedHeader information.
     * Update router's LocationTable and neighbour relationship.
     *
     * \param pdu containing the ExtendedHeader
     * \param packet received packet
     */
    void process_extended(const ExtendedPduConstRefs<BeaconHeader>&, UpPacketPtr);

    /**
     * \brief Process ExtendedHeader information.
     * Update router's LocationTable and neighbour relationship.
     * Pass packet up to transport layer for further processing.
     *
     * \param pdu containing the ExtendedHeader
     * \param packet received packet
     */
    void process_extended(const ExtendedPduConstRefs<ShbHeader>&, UpPacketPtr);

    /**
     * \brief Process ExtendedHeader information.
     * Update router's LocationTable and neighbour relationship.
     * Pass packet up to transport layer for further processing.
     * Forward packets.
     *
     * \param pdu containing the ExtendedHeader
     * \param packet received packet
     * \param sender
     * \param destination
     */
    void process_extended(const ExtendedPduConstRefs<GeoBroadcastHeader>&, UpPacketPtr,
            const MacAddress& sender, const MacAddress& destination);

    /**
     * \brief Send all buffered packet whose waiting time expired.
     *
     * \param buffer
     */
    void flush_forwarding_buffer(PacketBuffer&);

    /**
     * \brief Send all packets in the broadcast forwarding buffer with expired waiting time.
     */
    void flush_broadcast_forwarding_buffer();

    /**
     * \brief Send all packets in the unicast forwarding buffer with expired waiting time.
     */
    void flush_unicast_forwarding_buffer();

    /**
     * \brief Executes media specific functionalities
     * Details are described in TS 102 636-4-2.
     *
     * \param profile e.g. ITS-G5
     */
    void execute_media_procedures(CommunicationProfile);

    /**
     * \brief Executes ITS-G5 media specific procedures
     * Details are described in TS 102 636-4-2.
     */
    void execute_itsg5_procedures();

    /**
     * \brief Pass down the packet to the access layer.
     *
     * \param addr MAC address of destination
     * \param pdu header information
     * \param payload Packet payload
     */
    void pass_down(const MacAddress&, PduPtr, DownPacketPtr);

    /**
     * \brief Send packet using the information in the DataRequest.
     * The packet is formed using the data in PDU and payload.
     *
     * \param request containing transmission parameters
     * \param pdu header information
     * \param payload Packet payload
     */
    void pass_down(const dcc::DataRequest&, PduPtr, DownPacketPtr);

    /**
     * \brief Pass packet up to the transport layer.
     *
     * \param ind containing network information
     * \param packet payload to be passed up to the next layer
     */
    void pass_up(DataIndication&, UpPacketPtr);

    /**
     * \brief Helper method to handle duplicate addresses.
     * If own address collides with the address of a received packet
     * Router's address is set to a new random address.
     * \note Behaviour depends on MIB's itsGnLocalAddrConfMethod.
     *
     * \param addr_so address to be examined
     */
    void detect_duplicate_address(const Address&);

    /**
     * \brief Determine next hop for GBC Advanced forwarding.
     * See TS 102 636-4 v1.2.3 E.4
     *
     * \param scf Store & Carry Forwarding
     * \param sender
     * \param destination
     * \param pdu
     * \param payload
     * \return next hop
     */
    NextHop next_hop_gbc_advanced(bool scf, const MacAddress& sender, const MacAddress& destination,
            std::unique_ptr<GbcPdu>, DownPacketPtr);

    /**
     * \brief Determine first hop for GBC Advanced forwarding.
     *
     * \param scf Store & Carry Forwarding
     * \param pdu
     * \param payload
     * \return first hop
     */
    NextHop first_hop_gbc_advanced(bool scf, std::unique_ptr<GbcPdu>, DownPacketPtr);

    /**
     * \brief Determine next hop for contention-based forwarding.
     * See TS 102 636-4 v1.2.3 E.3
     *
     * \param scf Store & Carry Forwarding
     * \param sender
     * \param pdu
     * \param payload
     * \return next hop
     */
    NextHop next_hop_contention_based_forwarding(bool scf, const MacAddress& sender,
            std::unique_ptr<GbcPdu>, DownPacketPtr);

    /**
     * \brief Determine next hop for greedy forwarding.
     * See TS 102 636-4 v1.2.3 D.2
     *
     * \param scf Store & Carry Forwarding
     * \param pdu
     * \param payload
     * \return next hop
     */
    NextHop next_hop_greedy_forwarding(bool scf, std::unique_ptr<GbcPdu>, DownPacketPtr);

    /**
     * \brief Determine first hop for contention-based forwarding.
     *
     * \param scf Store & Carry Forwarding
     * \param pdu
     * \param payload
     * \return first hop
     */
    NextHop first_hop_contention_based_forwarding(bool scf, std::unique_ptr<GbcPdu>, DownPacketPtr);

    /**
     * \brief Determine CBF buffering time for a GBC packet.
     * See TS 102 636-4 v1.2.3 E.3
     *
     * \param dist Distance between local router and sender
     * \return CBF time-out
     */
    units::Duration timeout_cbf_gbc(units::Length distance) const;

    /**
     * \brief Determine CBF buffering time for a GBC packet.
     * See TS 102 636-4 v1.2.3 E.3
     *
     * \param sender MAC address of sender
     * \return CBF time-out
     */
    units::Duration timeout_cbf_gbc(const MacAddress& sender) const;

    /**
     * \brief Callback function for dispatching a packet repetition.
     * Invoked by Repeater when a scheduled repetition is due.
     *
     * \param request
     * \param payload
     */
    void dispatch_repetition(const DataRequestVariant&, DownPacketPtr);

    /**
     * \brief Encaspulate a packet according to security profile
     *
     * \param profile Security profile
     * \param pdu PDU
     * \param packet Packet with payload
     */
    DownPacketPtr encap_packet(security::Profile, Pdu&, DownPacketPtr);

    const MIB& m_mib;
    Clock::time_point m_clock;
    dcc::RequestInterface& m_request_interface;
    security::SecurityEntity m_security_entity;
    transport_map_t m_transport_ifcs;
    LocationTable m_location_table;
    PacketBuffer m_bc_forward_buffer;
    PacketBuffer m_uc_forward_buffer;
    CbfPacketBuffer m_cbf_buffer;
    LongPositionVector m_local_position_vector;
    SequenceNumber m_local_sequence_number;
    Repeater m_repeater;
    Timestamp m_last_update_lpv;
    Timestamp m_time_now;
    Timestamp m_last_transmission;
    Timestamp m_next_beacon;
    std::mt19937 m_random_gen;
};

} // namespace geonet
} // namespace vanetza

#endif /* ROUTER_HPP_UKYYCAR0 */
