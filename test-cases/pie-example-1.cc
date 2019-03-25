/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 NITK Surathkal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Vivek Jain <jain.vivek.anand@gmail.com>
 *          Viyom Mittal <viyommittal@gmail.com>
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 */

#include <iostream>
#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/applications-module.h"
#include "ns3/packet-sink.h"
#include "ns3/traffic-control-module.h"
#include "ns3/log.h"
#include "ns3/random-variable-stream.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/callback.h"

using namespace ns3;
Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
std::string dir = "/home/siddeshlc8/siddeshlc/NS3/ns-allinone-3.29/ns-3.29/results/pie/";
double stopTime = 100;
uint32_t useTsp = 0;
uint32_t checkTimes;
double avgQueueDiscSize;

void
CheckQueueSize (Ptr<QueueDisc> queue)
{
  uint32_t qSize = queue->GetCurrentSize ().GetValue ();

  avgQueueDiscSize += qSize;
  checkTimes++;

  std::string ext = "_UseTsp";
  if(!useTsp)
    ext = "";

  // check queue size every 1/100 of a second
  Simulator::Schedule (Seconds (0.001), &CheckQueueSize, queue);

  std::ofstream fPlotQueue1 (std::stringstream (dir + "inst-queue-size"+ext+".plotme").str ().c_str (), std::ios::out | std::ios::app);
  fPlotQueue1 << Simulator::Now ().GetSeconds () << " " << qSize << std::endl;
  fPlotQueue1.close ();

  std::ofstream fPlotQueue2 (std::stringstream (dir + "avg-queue-size"+ext+".plotme").str ().c_str (), std::ios::out | std::ios::app);
  fPlotQueue2 << Simulator::Now ().GetSeconds () << " " << avgQueueDiscSize/checkTimes << std::endl;
  fPlotQueue2.close ();
}

static void
CwndChangeA (uint32_t oldCwnd, uint32_t newCwnd)
{
  std::ofstream fPlotQueue (dir + "cwndTraces/A.plotme", std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << newCwnd/1446.0 << std::endl;
  fPlotQueue.close ();
}

static void
CwndChangeB (uint32_t oldCwnd, uint32_t newCwnd)
{
  std::ofstream fPlotQueue (dir + "cwndTraces/B.plotme", std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << newCwnd/1446.0 << std::endl;
  fPlotQueue.close ();
}

static void
CwndChangeC (uint32_t oldCwnd, uint32_t newCwnd)
{
  std::ofstream fPlotQueue (dir + "cwndTraces/C.plotme", std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << newCwnd/1446.0 << std::endl;
  fPlotQueue.close ();
}

static void
CwndChangeD (uint32_t oldCwnd, uint32_t newCwnd)
{
  std::ofstream fPlotQueue (dir + "cwndTraces/D.plotme", std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << newCwnd/1446.0 << std::endl;
  fPlotQueue.close ();
}

static void
CwndChangeE (uint32_t oldCwnd, uint32_t newCwnd)
{
  std::ofstream fPlotQueue (dir + "cwndTraces/E.plotme", std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << newCwnd/1446.0 << std::endl;
  fPlotQueue.close ();
}


void
TraceCwnd (uint32_t node, uint32_t cwndWindow,
           Callback <void, uint32_t, uint32_t> CwndTrace)
{
  Config::ConnectWithoutContext ("/NodeList/" + std::to_string (node) + "/$ns3::TcpL4Protocol/SocketList/" + std::to_string (cwndWindow) + "/CongestionWindow", CwndTrace);
}

void InstallPacketSink (Ptr<Node> node, uint16_t port)
{
  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (node);
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (stopTime));
}

void InstallBulkSend (Ptr<Node> node, Ipv4Address address, uint16_t port, 
                      uint32_t nodeId, uint32_t cwndWindow,
                      Callback <void, uint32_t, uint32_t> CwndTrace)
{
  BulkSendHelper source ("ns3::TcpSocketFactory", 
                         InetSocketAddress (address, port));

  source.SetAttribute ("MaxBytes", UintegerValue (0));
  ApplicationContainer sourceApps = source.Install (node);
  Time timeToStart = Seconds (uv->GetValue (0, 1));
  sourceApps.Start (timeToStart);
  Simulator::Schedule (timeToStart + Seconds (0.001), &TraceCwnd, nodeId, cwndWindow, CwndTrace);
  sourceApps.Stop (Seconds (stopTime));
}

int main (int argc, char *argv[])
{
  uint32_t stream = 1;
  std::string transport_prot = "TcpNewReno";
  std::string queue_disc_type = "PieQueueDisc";
  uint32_t dataSize = 1446;
  uint32_t delAckCount = 2;


  CommandLine cmd;
  cmd.AddValue ("useTsp", "Use TimeStamp", useTsp);
  cmd.Parse (argc,argv);

  uv->SetStream (stream);
  transport_prot = std::string ("ns3::") + transport_prot;
  queue_disc_type = std::string ("ns3::") + queue_disc_type;

  TypeId qdTid;
  NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (queue_disc_type, &qdTid), "TypeId " << queue_disc_type << " not found");

  TypeId tcpTid;
  NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (transport_prot, &tcpTid), "TypeId " << transport_prot << " not found");
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (transport_prot)));

  Config::SetDefault ("ns3::TcpL4Protocol::RecoveryType", TypeIdValue (TypeId::LookupByName ("ns3::TcpPrrRecovery")));

  NodeContainer leftNodes, rightNodes, routers;
  routers.Create (2);
  leftNodes.Create (5);
  rightNodes.Create (5);

  // Create the point-to-point link helpers
  PointToPointHelper pointToPointRouter;
  pointToPointRouter.SetDeviceAttribute  ("DataRate", StringValue ("5Mbps"));
  pointToPointRouter.SetChannelAttribute ("Delay", StringValue ("0.00075ms"));
  NetDeviceContainer r1r2ND = pointToPointRouter.Install (routers.Get (0), routers.Get (1));

  std::vector <NetDeviceContainer> leftToRouter;
  std::vector <NetDeviceContainer> routerToRight;
  PointToPointHelper pointToPointLeaf;
  pointToPointLeaf.SetDeviceAttribute    ("DataRate", StringValue ("5Mbps"));

  // Node 1
  pointToPointLeaf.SetChannelAttribute   ("Delay", StringValue ("0.00025ms"));
  leftToRouter.push_back (pointToPointLeaf.Install (leftNodes.Get (0), routers.Get (0)));
  routerToRight.push_back (pointToPointLeaf.Install (routers.Get (1), rightNodes.Get (0)));

  // Node 2
  pointToPointLeaf.SetChannelAttribute   ("Delay", StringValue ("0.0001ms"));
  leftToRouter.push_back (pointToPointLeaf.Install (leftNodes.Get (1), routers.Get (0)));
  routerToRight.push_back (pointToPointLeaf.Install (routers.Get (1), rightNodes.Get (1)));

  // Node 3
  pointToPointLeaf.SetChannelAttribute   ("Delay", StringValue ("0.00005ms"));
  leftToRouter.push_back (pointToPointLeaf.Install (leftNodes.Get (2), routers.Get (0)));
  routerToRight.push_back (pointToPointLeaf.Install (routers.Get (1), rightNodes.Get (2)));

  // Node 4
  pointToPointLeaf.SetChannelAttribute   ("Delay", StringValue ("0.000025ms"));
  leftToRouter.push_back (pointToPointLeaf.Install (leftNodes.Get (3), routers.Get (0)));
  routerToRight.push_back (pointToPointLeaf.Install (routers.Get (1), rightNodes.Get (3)));

  // Node 5
  pointToPointLeaf.SetChannelAttribute   ("Delay", StringValue ("0.000005ms"));
  leftToRouter.push_back (pointToPointLeaf.Install (leftNodes.Get (4), routers.Get (0)));
  routerToRight.push_back (pointToPointLeaf.Install (routers.Get (1), rightNodes.Get (4)));

  // Install Stack
  InternetStackHelper stack;
  stack.Install (routers);
  stack.Install (leftNodes);
  stack.Install (rightNodes);

  Ipv4AddressHelper ipAddresses ("10.0.0.0", "255.255.255.0");

  Ipv4InterfaceContainer r1r2IPAddress = ipAddresses.Assign (r1r2ND);        ipAddresses.NewNetwork ();

  std::vector <Ipv4InterfaceContainer> leftToRouterIPAddress;
  leftToRouterIPAddress.push_back (ipAddresses.Assign (leftToRouter [0]));   ipAddresses.NewNetwork ();
  leftToRouterIPAddress.push_back (ipAddresses.Assign (leftToRouter [1]));   ipAddresses.NewNetwork ();
  leftToRouterIPAddress.push_back (ipAddresses.Assign (leftToRouter [2]));   ipAddresses.NewNetwork ();
  leftToRouterIPAddress.push_back (ipAddresses.Assign (leftToRouter [3]));   ipAddresses.NewNetwork ();
  leftToRouterIPAddress.push_back (ipAddresses.Assign (leftToRouter [4]));   ipAddresses.NewNetwork ();

  std::vector <Ipv4InterfaceContainer> routerToRightIPAddress;
  routerToRightIPAddress.push_back (ipAddresses.Assign (routerToRight [0]));   ipAddresses.NewNetwork ();
  routerToRightIPAddress.push_back (ipAddresses.Assign (routerToRight [1]));   ipAddresses.NewNetwork ();
  routerToRightIPAddress.push_back (ipAddresses.Assign (routerToRight [2]));   ipAddresses.NewNetwork ();
  routerToRightIPAddress.push_back (ipAddresses.Assign (routerToRight [3]));   ipAddresses.NewNetwork ();
  routerToRightIPAddress.push_back (ipAddresses.Assign (routerToRight [4]));

  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (1 << 20));
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (1 << 20));
  Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (10));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (delAckCount));
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (dataSize));

  uint32_t meanPktSize = 1000;

  Config::SetDefault ("ns3::PieQueueDisc::MaxSize", StringValue ("100p"));
  Config::SetDefault ("ns3::PieQueueDisc::MeanPktSize", UintegerValue (meanPktSize));
  Config::SetDefault ("ns3::PieQueueDisc::DequeueThreshold", UintegerValue (10000));
  Config::SetDefault ("ns3::PieQueueDisc::QueueDelayReference", TimeValue (Seconds (0.2)));
  Config::SetDefault ("ns3::PieQueueDisc::MaxBurstAllowance", TimeValue (Seconds (0.1)));
  Config::SetDefault ("ns3::PieQueueDisc::UseTimeStamp", UintegerValue (useTsp));

  AsciiTraceHelper asciiTraceHelper;

  TrafficControlHelper tch;
  tch.SetRootQueueDisc (queue_disc_type);
  QueueDiscContainer qd;
  tch.Uninstall (routers.Get (0)->GetDevice (0));
  qd.Add (tch.Install (routers.Get (0)->GetDevice (0)).Get (0));
  Simulator::ScheduleNow (&CheckQueueSize, qd.Get (0));

  uint16_t port = 50000;
  InstallPacketSink (rightNodes.Get (0), port);      // A Sink 0 Applications
  InstallPacketSink (rightNodes.Get (1), port);      // B Sink 0 Applications
  InstallPacketSink (rightNodes.Get (2), port);      // C Sink 0 Applications
  InstallPacketSink (rightNodes.Get (3), port);      // D Sink 0 Applications
  InstallPacketSink (rightNodes.Get (4), port);      // E Sink 0 Applications

  InstallBulkSend (leftNodes.Get (0), routerToRightIPAddress [0].GetAddress (1), port, 2, 0, MakeCallback (&CwndChangeA));
  InstallBulkSend (leftNodes.Get (1), routerToRightIPAddress [1].GetAddress (1), port, 3, 0, MakeCallback (&CwndChangeB));
  InstallBulkSend (leftNodes.Get (2), routerToRightIPAddress [2].GetAddress (1), port, 4, 0, MakeCallback (&CwndChangeC));
  InstallBulkSend (leftNodes.Get (3), routerToRightIPAddress [3].GetAddress (1), port, 5, 0, MakeCallback (&CwndChangeD));
  InstallBulkSend (leftNodes.Get (4), routerToRightIPAddress [4].GetAddress (1), port, 6, 0, MakeCallback (&CwndChangeE));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (stopTime));
  Simulator::Run ();

  Simulator::Destroy ();
  return 0;
}