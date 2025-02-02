/***********************************************************************************************************************
*  OpenStudio(R), Copyright (c) Alliance for Sustainable Energy, LLC.
*  See also https://openstudio.net/license
***********************************************************************************************************************/

#include <gtest/gtest.h>
#include "EnergyPlusFixture.hpp"

#include "../ErrorFile.hpp"
#include "../ForwardTranslator.hpp"
#include "../ReverseTranslator.hpp"

#include "../../model/Model.hpp"
#include "../../model/Building.hpp"
#include "../../model/Building_Impl.hpp"
#include "../../model/Site.hpp"
#include "../../model/Site_Impl.hpp"
#include "../../model/WeatherFile.hpp"
#include "../../model/WeatherFile_Impl.hpp"
#include "../../model/ThermalZone.hpp"
#include "../../model/Component.hpp"
#include "../../model/Curve.hpp"
#include "../../model/Curve_Impl.hpp"
#include "../../model/CurveQuadratic.hpp"
#include "../../model/CurveQuadratic_Impl.hpp"

#include "../../model/GeneratorFuelCell.hpp"
#include "../../model/GeneratorFuelCellAirSupply.hpp"
#include "../../model/GeneratorFuelCellAuxiliaryHeater.hpp"
#include "../../model/GeneratorFuelCellElectricalStorage.hpp"
#include "../../model/GeneratorFuelCellExhaustGasToWaterHeatExchanger.hpp"
#include "../../model/GeneratorFuelCellInverter.hpp"
#include "../../model/GeneratorFuelCellPowerModule.hpp"
#include "../../model/GeneratorFuelCellStackCooler.hpp"
#include "../../model/GeneratorFuelCellWaterSupply.hpp"
#include "../../model/GeneratorFuelSupply.hpp"
#include "../../model/GeneratorFuelCell_Impl.hpp"
#include "../../model/GeneratorFuelCellAirSupply_Impl.hpp"
#include "../../model/GeneratorFuelCellAuxiliaryHeater_Impl.hpp"
#include "../../model/GeneratorFuelCellElectricalStorage_Impl.hpp"
#include "../../model/GeneratorFuelCellExhaustGasToWaterHeatExchanger_Impl.hpp"
#include "../../model/GeneratorFuelCellInverter_Impl.hpp"
#include "../../model/GeneratorFuelCellPowerModule_Impl.hpp"
#include "../../model/GeneratorFuelCellStackCooler_Impl.hpp"
#include "../../model/GeneratorFuelCellWaterSupply_Impl.hpp"
#include "../../model/GeneratorFuelSupply_Impl.hpp"
#include "../../model/ElectricLoadCenterDistribution.hpp"
#include "../../model/ElectricLoadCenterDistribution_Impl.hpp"

#include "../../model/Version.hpp"
#include "../../model/Version_Impl.hpp"

#include "../../utilities/geometry/Point3d.hpp"

#include "../../utilities/core/Optional.hpp"
#include "../../utilities/core/Checksum.hpp"
#include "../../utilities/core/UUID.hpp"
#include "../../utilities/core/Logger.hpp"
#include "../../utilities/sql/SqlFile.hpp"
#include "../../utilities/idf/IdfFile.hpp"
#include "../../utilities/idf/IdfObject.hpp"

#include <utilities/idd/OS_Generator_FuelCell_AirSupply_FieldEnums.hxx>
#include <utilities/idd/Generator_FuelCell_AirSupply_FieldEnums.hxx>
#include <utilities/idd/OS_Generator_FuelCell_AuxiliaryHeater_FieldEnums.hxx>
#include <utilities/idd/Generator_FuelCell_AuxiliaryHeater_FieldEnums.hxx>
#include <utilities/idd/OS_Generator_FuelCell_ElectricalStorage_FieldEnums.hxx>
#include <utilities/idd/Generator_FuelCell_ElectricalStorage_FieldEnums.hxx>
#include <utilities/idd/OS_Generator_FuelCell_ExhaustGasToWaterHeatExchanger_FieldEnums.hxx>
#include <utilities/idd/Generator_FuelCell_ExhaustGasToWaterHeatExchanger_FieldEnums.hxx>
#include <utilities/idd/OS_Generator_FuelCell_FieldEnums.hxx>
#include <utilities/idd/Generator_FuelCell_FieldEnums.hxx>
#include <utilities/idd/OS_Generator_FuelCell_Inverter_FieldEnums.hxx>
#include <utilities/idd/Generator_FuelCell_Inverter_FieldEnums.hxx>
#include <utilities/idd/OS_Generator_FuelCell_PowerModule_FieldEnums.hxx>
#include <utilities/idd/Generator_FuelCell_PowerModule_FieldEnums.hxx>
#include <utilities/idd/OS_Generator_FuelCell_StackCooler_FieldEnums.hxx>
#include <utilities/idd/Generator_FuelCell_StackCooler_FieldEnums.hxx>
#include <utilities/idd/OS_Generator_FuelCell_WaterSupply_FieldEnums.hxx>
#include <utilities/idd/Generator_FuelCell_WaterSupply_FieldEnums.hxx>
#include <utilities/idd/OS_Generator_FuelSupply_FieldEnums.hxx>
#include <utilities/idd/Generator_FuelSupply_FieldEnums.hxx>
#include <utilities/idd/OS_ElectricLoadCenter_Distribution_FieldEnums.hxx>
#include <utilities/idd/ElectricLoadCenter_Distribution_FieldEnums.hxx>
#include <utilities/idd/ElectricLoadCenter_Generators_FieldEnums.hxx>
#include <utilities/idd/IddEnums.hxx>
#include <utilities/idd/IddFactory.hxx>

#include <boost/algorithm/string/predicate.hpp>

#include <resources.hxx>

#include <sstream>

#include <vector>

using namespace openstudio::energyplus;
using namespace openstudio::model;
using namespace openstudio;

TEST_F(EnergyPlusFixture, ForwardTranslatorFuelCell) {

  Model model;

  Building building = model.getUniqueModelObject<Building>();

  //ThermalZone zone1(model);
  //ThermalZone zone2(model);

  // create default fuelcell
  GeneratorFuelCell fuelcell(model);
  // get default power module
  GeneratorFuelCellPowerModule fCPM = fuelcell.powerModule();
  // check default power module curve values
  Curve curve = fCPM.efficiencyCurve();
  auto curveQ = curve.cast<CurveQuadratic>();
  EXPECT_EQ(0.642388, curveQ.coefficient1Constant());
  EXPECT_EQ(-0.0001619, curveQ.coefficient2x());
  EXPECT_EQ(2.26e-008, curveQ.coefficient3xPOW2());
  EXPECT_EQ("Annex42", fCPM.efficiencyCurveMode());

  // check default Airsupply
  GeneratorFuelCellAirSupply fAS = fuelcell.airSupply();
  EXPECT_EQ("AirRatiobyStoics", fAS.airSupplyRateCalculationMode());
  EXPECT_EQ(1.0, fAS.stoichiometricRatio().get());
  EXPECT_EQ("NoRecovery", fAS.airIntakeHeatRecoveryMode());
  EXPECT_EQ("AmbientAir", fAS.airSupplyConstituentMode());

  // check default fuel supply
  GeneratorFuelSupply fS = fuelcell.fuelSupply();

  // check default water supply
  GeneratorFuelCellWaterSupply fWS = fuelcell.waterSupply();
  fWS.setWaterTemperatureModelingMode("MainsWaterTemperature");

  // check default aux heater
  GeneratorFuelCellAuxiliaryHeater fAX = fuelcell.auxiliaryHeater();

  // check default heat exchanger
  GeneratorFuelCellExhaustGasToWaterHeatExchanger fHX = fuelcell.heatExchanger();

  // check default electric storage
  GeneratorFuelCellElectricalStorage fES = fuelcell.electricalStorage();

  // check default inverter
  GeneratorFuelCellInverter fI = fuelcell.inverter();

  // check default optional stackcooler
  boost::optional<GeneratorFuelCellStackCooler> fSC = fuelcell.stackCooler();
  EXPECT_FALSE(fSC);

  ElectricLoadCenterDistribution elcd(model);
  elcd.setElectricalBussType("AlternatingCurrent");
  elcd.addGenerator(fuelcell);

  ForwardTranslator forwardTranslator;
  Workspace workspace = forwardTranslator.translateModel(model);

  EXPECT_EQ(0u, forwardTranslator.errors().size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_AirSupply).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_AuxiliaryHeater).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_ElectricalStorage).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_ExhaustGasToWaterHeatExchanger).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_Inverter).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_PowerModule).size());
  // no stack cooler by default
  EXPECT_EQ(0u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_StackCooler).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_WaterSupply).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelSupply).size());
  //expect site water mains
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Site_WaterMainsTemperature).size());

  // model.save(toPath("./fuelcell.osm"), true);
  // workspace.save(toPath("./fuelcell.idf"), true);
}

TEST_F(EnergyPlusFixture, ForwardTranslatorFuelCell2) {

  Model model;

  Building building = model.getUniqueModelObject<Building>();

  GeneratorFuelCellAirSupply airSupply(model);
  GeneratorFuelCellAuxiliaryHeater auxHeater(model);
  GeneratorFuelCellElectricalStorage elecStorage(model);
  GeneratorFuelCellExhaustGasToWaterHeatExchanger exhaustHX(model);
  GeneratorFuelCellInverter inverter(model);
  GeneratorFuelCellPowerModule powerModule(model);
  GeneratorFuelCellStackCooler stackCooler(model);
  GeneratorFuelCellWaterSupply waterSupply(model);
  GeneratorFuelSupply fuelSupply(model);
  // create default fuelcell
  GeneratorFuelCell fuelcell(model, powerModule, airSupply, waterSupply, auxHeater, exhaustHX, elecStorage, inverter, fuelSupply);
  // Stack cooler is optional. For it to be translated, it needs to be linked to a fuelcell parent
  fuelcell.setStackCooler(stackCooler);

  ElectricLoadCenterDistribution elcd(model);
  elcd.setElectricalBussType("AlternatingCurrent");
  elcd.addGenerator(fuelcell);

  ForwardTranslator forwardTranslator;
  Workspace workspace = forwardTranslator.translateModel(model);

  EXPECT_EQ(0u, forwardTranslator.errors().size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::ElectricLoadCenter_Generators).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::ElectricLoadCenter_Distribution).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_AirSupply).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_AuxiliaryHeater).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_ElectricalStorage).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_ExhaustGasToWaterHeatExchanger).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_Inverter).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_PowerModule).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_WaterSupply).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelSupply).size());
  //no water mains expected
  EXPECT_EQ(0u, workspace.getObjectsByType(IddObjectType::Site_WaterMainsTemperature).size());
  // Expect stack cooler since it is linked to a FuelCell
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_StackCooler).size());

  // model.save(toPath("./fuelcell2.osm"), true);
  // workspace.save(toPath("./fuelcell2.idf"), true);
}

TEST_F(EnergyPlusFixture, ForwardTranslatorFuelCell3) {

  Model model;

  Building building = model.getUniqueModelObject<Building>();

  GeneratorFuelCellAirSupply airSupply(model);
  GeneratorFuelCellAuxiliaryHeater auxHeater(model);
  GeneratorFuelCellElectricalStorage elecStorage(model);
  GeneratorFuelCellExhaustGasToWaterHeatExchanger exhaustHX(model);
  GeneratorFuelCellInverter inverter(model);
  GeneratorFuelCellPowerModule powerModule(model);
  GeneratorFuelCellStackCooler stackCooler(model);
  GeneratorFuelCellWaterSupply waterSupply(model);
  GeneratorFuelSupply fuelSupply(model);
  // create default fuelcell
  GeneratorFuelCell fuelcell(model, powerModule, airSupply, waterSupply, auxHeater, exhaustHX, elecStorage, inverter, fuelSupply);

  // Create an ELCD
  ElectricLoadCenterDistribution elcd = ElectricLoadCenterDistribution(model);
  elcd.setName("fuelcell ELCD");
  elcd.setElectricalBussType("AlternatingCurrent");
  elcd.addGenerator(fuelcell);

  ForwardTranslator forwardTranslator;
  Workspace workspace = forwardTranslator.translateModel(model);

  EXPECT_EQ(0u, forwardTranslator.errors().size());

  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::ElectricLoadCenter_Generators).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::ElectricLoadCenter_Distribution).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_AirSupply).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_AuxiliaryHeater).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_ElectricalStorage).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_ExhaustGasToWaterHeatExchanger).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_Inverter).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_PowerModule).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_WaterSupply).size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::Generator_FuelSupply).size());
  //no water mains expected
  EXPECT_EQ(0u, workspace.getObjectsByType(IddObjectType::Site_WaterMainsTemperature).size());
  // Stack cooler is optional. For it to be transalted, it needs to be linked to a fuelcell parent
  // Here we didn't, so we don't expect a Stack Cooler
  EXPECT_EQ(0u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_StackCooler).size());

  // model.save(toPath("./fuelcell3.osm"), true);
  // workspace.save(toPath("./fuelcell3.idf"), true);
}

TEST_F(EnergyPlusFixture, ForwardTranslatorFuelCell4) {
  //TEST orphaned FC

  Model model;

  Building building = model.getUniqueModelObject<Building>();

  GeneratorFuelCellAirSupply airSupply(model);
  GeneratorFuelCellAuxiliaryHeater auxHeater(model);
  GeneratorFuelCellElectricalStorage elecStorage(model);
  GeneratorFuelCellExhaustGasToWaterHeatExchanger exhaustHX(model);
  GeneratorFuelCellInverter inverter(model);
  GeneratorFuelCellPowerModule powerModule(model);
  //GeneratorFuelCellStackCooler stackCooler(model);
  GeneratorFuelCellWaterSupply waterSupply(model);
  GeneratorFuelSupply fuelSupply(model);
  // create default fuelcell
  GeneratorFuelCell fuelcell(model, powerModule, airSupply, waterSupply, auxHeater, exhaustHX, elecStorage, inverter, fuelSupply);

  // check the ELCD
  boost::optional<ElectricLoadCenterDistribution> elcd = fuelcell.electricLoadCenterDistribution();
  EXPECT_FALSE(fuelcell.electricLoadCenterDistribution());

  ForwardTranslator forwardTranslator;
  Workspace workspace = forwardTranslator.translateModel(model);
  EXPECT_EQ(0u, forwardTranslator.errors().size());
  EXPECT_EQ(0u, forwardTranslator.warnings().size());

  //NO FC components should FT now since it is orphaned
  EXPECT_EQ(0u, workspace.getObjectsByType(IddObjectType::ElectricLoadCenter_Generators).size());
  EXPECT_EQ(0u, workspace.getObjectsByType(IddObjectType::ElectricLoadCenter_Distribution).size());
  EXPECT_EQ(0u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell).size());
  EXPECT_EQ(0u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_AirSupply).size());
  EXPECT_EQ(0u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_AuxiliaryHeater).size());
  EXPECT_EQ(0u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_ElectricalStorage).size());
  EXPECT_EQ(0u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_ExhaustGasToWaterHeatExchanger).size());
  EXPECT_EQ(0u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_Inverter).size());
  EXPECT_EQ(0u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_PowerModule).size());
  EXPECT_EQ(0u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_StackCooler).size());
  EXPECT_EQ(0u, workspace.getObjectsByType(IddObjectType::Generator_FuelCell_WaterSupply).size());
  EXPECT_EQ(0u, workspace.getObjectsByType(IddObjectType::Generator_FuelSupply).size());
  //no water mains expected
  EXPECT_EQ(0u, workspace.getObjectsByType(IddObjectType::Site_WaterMainsTemperature).size());

  // model.save(toPath("./fuelcell4.osm"), true);
  // workspace.save(toPath("./fuelcell4.idf"), true);
}
