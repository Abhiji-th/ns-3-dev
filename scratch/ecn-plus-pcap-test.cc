/*
 *
 * Simulation to test ECN+ (RFC 5562) SYN-ACK ECT marking with PCAP output in NS-3.
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/pcap-file.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("EcnPlusPcapTest");

int
main(int argc, char* argv[])
{
    // Enable logging for debugging
    LogComponentEnable("EcnPlusPcapTest", LOG_LEVEL_INFO);
    LogComponentEnable("TcpSocketBase", LOG_LEVEL_INFO);

    // Create two nodes: sender and receiver
    NodeContainer nodes;
    nodes.Create(2);

    // Set up a point-to-point link
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    // Install Internet stack
    InternetStackHelper stack;
    stack.Install(nodes);

    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // Create a TCP sink (receiver)
    uint16_t port = 8080;
    Address sinkAddress(InetSocketAddress(interfaces.GetAddress(1), port));
    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkAddress);
    ApplicationContainer sinkApp = sinkHelper.Install(nodes.Get(1));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(10.0));

    // Create a TCP sender
    OnOffHelper senderHelper("ns3::TcpSocketFactory", sinkAddress);
    senderHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
    senderHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
    senderHelper.SetAttribute("DataRate", DataRateValue(DataRate("1Mbps")));
    senderHelper.SetAttribute("PacketSize", UintegerValue(512));

    ApplicationContainer senderApp = senderHelper.Install(nodes.Get(0));
    senderApp.Start(Seconds(1.0));
    senderApp.Stop(Seconds(10.0));

    // Enable PCAP tracing
    pointToPoint.EnablePcapAll("ecn-plus-test-classicecnplus", false);

    // Run the simulation
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();

    NS_LOG_INFO("Simulation completed. Check ecn-plus-test-1-0.pcap for SYN-ACK ECT marking.");

    return 0;
}