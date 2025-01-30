/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Sustainable Energy, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#include <gtest/gtest.h>
#include "EnergyPlusFixture.hpp"

#include "../ForwardTranslator.hpp"

#include "../../model/Model.hpp"
#include "../../model/AirTerminalSingleDuctSeriesPIUReheat.hpp"
#include "../../model/Schedule.hpp"
#include "../../model/CoilHeatingElectric.hpp"
#include "../../model/ThermalZone.hpp"
#include "../../model/Space.hpp"
#include "../../model/AirLoopHVAC.hpp"
#include "../../model/Node.hpp"
#include "../../model/FanSystemModel.hpp"

#include <utilities/idd/IddObject.hpp>
#include <utilities/idd/AirTerminal_SingleDuct_SeriesPIU_Reheat_FieldEnums.hxx>
#include <utilities/idd/IddEnums.hxx>

using namespace openstudio::energyplus;
using namespace openstudio::model;
using namespace openstudio;

TEST_F(EnergyPlusFixture, ForwardTranslator_AirTerminalSingleDuctSeriesPIUReheat) {
  Model m;

  Schedule sch = m.alwaysOnDiscreteSchedule();
  FanSystemModel fan(m);
  CoilHeatingElectric coil(m);
  AirTerminalSingleDuctSeriesPIUReheat atu(m, fan, coil);

  EXPECT_TRUE(atu.setAvailabilitySchedule(sch));
  EXPECT_TRUE(atu.setMaximumAirFlowRate(1));
  EXPECT_TRUE(atu.setMaximumPrimaryAirFlowRate(2));
  EXPECT_TRUE(atu.setMinimumPrimaryAirFlowFraction(0.1));
  EXPECT_TRUE(atu.setMaximumHotWaterorSteamFlowRate(3));
  EXPECT_TRUE(atu.setMinimumHotWaterorSteamFlowRate(4));
  EXPECT_TRUE(atu.setConvergenceTolerance(5));
  EXPECT_TRUE(atu.setFanControlType("VariableSpeed"));
  EXPECT_TRUE(atu.setMinimumFanTurnDownRatio(0.2));
  EXPECT_TRUE(atu.setHeatingControlType("Modulated"));
  EXPECT_TRUE(atu.setDesignHeatingDischargeAirTemperature(6));
  EXPECT_TRUE(atu.setHighLimitHeatingDischargeAirTemperature(7));

  ThermalZone z(m);
  Space s(m);
  s.setThermalZone(z);
  AirLoopHVAC a(m);
  a.addBranchForZone(z, atu);

  ForwardTranslator ft;
  Workspace w = ft.translateModel(m);

  WorkspaceObjectVector idf_atus(w.getObjectsByType(IddObjectType::AirTerminal_SingleDuct_SeriesPIU_Reheat));
  ASSERT_EQ(1u, idf_atus.size());
  WorkspaceObject idf_atu(idf_atus[0]);

  EXPECT_EQ(atu.nameString(), idf_atu.getString(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::Name, false).get());
  EXPECT_EQ(sch.nameString(), idf_atu.getString(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::AvailabilityScheduleName, false).get());
  EXPECT_EQ(1, idf_atu.getDouble(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::MaximumAirFlowRate, false).get());
  EXPECT_EQ(2, idf_atu.getDouble(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::MaximumPrimaryAirFlowRate, false).get());
  EXPECT_EQ(0.1, idf_atu.getDouble(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::MinimumPrimaryAirFlowFraction, false).get());
  EXPECT_EQ(atu.inletModelObject()->nameString(),
            idf_atu.getString(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::SupplyAirInletNodeName, false).get());
  EXPECT_EQ(atu.secondaryAirInletNode()->nameString(),
            idf_atu.getString(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::SecondaryAirInletNodeName, false).get());
  EXPECT_EQ(atu.outletModelObject()->nameString(), idf_atu.getString(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::OutletNodeName, false).get());
  EXPECT_EQ(atu.nameString() + " Fan Outlet",
            idf_atu.getString(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::ReheatCoilAirInletNodeName, false).get());
  EXPECT_EQ(atu.nameString() + " Mixer", idf_atu.getString(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::ZoneMixerName, false).get());
  EXPECT_EQ(fan.nameString(), idf_atu.getString(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::FanName, false).get());
  EXPECT_EQ("Coil:Heating:Electric", idf_atu.getString(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::ReheatCoilObjectType, false).get());
  EXPECT_EQ(coil.nameString(), idf_atu.getString(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::ReheatCoilName, false).get());
  EXPECT_EQ(3, idf_atu.getDouble(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::MaximumHotWaterorSteamFlowRate, false).get());
  EXPECT_EQ(4, idf_atu.getDouble(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::MinimumHotWaterorSteamFlowRate, false).get());
  EXPECT_EQ(5, idf_atu.getDouble(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::ConvergenceTolerance, false).get());
  EXPECT_EQ("VariableSpeed", idf_atu.getString(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::FanControlType, false).get());
  EXPECT_EQ(0.2, idf_atu.getDouble(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::MinimumFanTurnDownRatio, false).get());
  EXPECT_EQ("Modulated", idf_atu.getString(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::HeatingControlType, false).get());
  EXPECT_EQ(6, idf_atu.getDouble(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::DesignHeatingDischargeAirTemperature, false).get());
  EXPECT_EQ(7, idf_atu.getDouble(AirTerminal_SingleDuct_SeriesPIU_ReheatFields::HighLimitHeatingDischargeAirTemperature, false).get());
}
