#pragma once
#include "../MachineTools/InstantiatedMachineTool.hpp"
#include "../TypeDefinition/GMS/GMSType.hpp"

class MitutoyoSim : public InstantiatedMachineTool {
 public:
  MitutoyoSim(UA_Server *pServer);

  void Simulate() override;
  virtual ~MitutoyoSim() = default;

 protected:
  void CreateObject() override;

  GMS::GMS_t mt;
  UA_String m_resulturi[1];
  void InstantiateIdentification();
  void InstantiateMonitoring();
  void InstantiateProduction();
  void InstantiateTools();
  int m_simStep = 0;

  void initCorrection(GMS::CorrectionType_t &corr, std::string Identifier, std::string CharacteristicIdentfier, double value);

  void InstantiateResultManagement();
};