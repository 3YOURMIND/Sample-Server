#include "MitutoyoSim.hpp"

#include <string>

#include "../TypeDefinition/GMS/Constants.hpp"
#include "../TypeDefinition/GMS/GMSType.hpp"

MitutoyoSim::MitutoyoSim(UA_Server *pServer) : InstantiatedMachineTool(pServer) {
  MachineName = "MitutoyoCMM";
  CreateObject();
}

void MitutoyoSim::CreateObject() {
  std::stringstream ss;
  ss << "https://www.mitutoyo.com/" << MachineName << "/";
  m_nsIndex = UA_Server_addNamespace(m_pServer, ss.str().c_str());
  UA_ObjectAttributes objAttr = UA_ObjectAttributes_default;

  objAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("", MachineName.c_str());
  objAttr.eventNotifier = UA_EVENTNOTIFIERTYPE_SUBSCRIBETOEVENTS;

  auto status = UA_Server_addObjectNode(
    m_pServer,
    UA_NODEID_NUMERIC(m_nsIndex, 0),
    UA_NODEID_NUMERIC(nsFromUri(m_pServer, constants::NsMachineryUri), UA_MACHINERYID_MACHINES),
    UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
    *open62541Cpp::UA_QualifiedName(m_nsIndex, MachineName).QualifiedName,
    UA_NODEID_NUMERIC(nsFromUri(m_pServer, constants::NsGMSUri), UA_GMSID_GMSTYPE),
    objAttr,
    nullptr,
    m_nodeId.NodeId);

  UA_ObjectAttributes_clear(&objAttr);
  UmatiServerLib::Bind::MembersRefl(mt, m_pServer, m_nodeId, n);
  InstantiateIdentification();
  InstantiateMonitoring();
  InstantiateProduction();
  InstantiateTools();
  InstantiateResultManagement();
}

void MitutoyoSim::InstantiateResultManagement() {}

void MitutoyoSim::InstantiateIdentification() {
  {
    std::stringstream ss;
    ss << "https://www.mitutoyo.com/GMS/#MitutoyoCMM";
    mt.Identification->ProductInstanceUri = ss.str();
  }

  InstantiateOptional(mt.Identification->YearOfConstruction, m_pServer, n);
  InstantiateOptional(mt.Identification->ProductCode, m_pServer, n);
  InstantiateOptional(mt.Identification->SoftwareRevision, m_pServer, n);
  InstantiateOptional(mt.Identification->DeviceClass, m_pServer, n);
  InstantiateOptional(mt.Identification->Location, m_pServer, n);
  InstantiateOptional(mt.Identification->Model, m_pServer, n);

  mt.Identification->Manufacturer = {"", "Mitutoyo Corporation"};
  mt.Identification->ProductCode = "Crysta Apex S 9108";
  mt.Identification->YearOfConstruction = 2022;
  mt.Identification->SoftwareRevision = "MCOSMOS 5.2.0";
  mt.Identification->DeviceClass = "CoordinateMeasuringMachine";
  mt.Identification->Location = "N 48.293869357852756 E 8.551257829466413";
  mt.Identification->Model = {"", MachineName};
  mt.Identification->SerialNumber = "0042";
}

void MitutoyoSim::InstantiateMonitoring() {
  InstantiateOptional(mt.Monitoring->MachineTool->PowerOnDuration, m_pServer, n);
  InstantiateOptional(mt.Monitoring->MachineTool->FeedOverride, m_pServer, n);
  mt.Monitoring->MachineTool->OperationMode = UA_MachineOperationMode::UA_MACHINEOPERATIONMODE_AUTOMATIC;
}

void MitutoyoSim::InstantiateProduction() {
  InstantiateOptional(mt.Production->ActiveProgram->State, m_pServer, n);
  mt.Production->ActiveProgram->NumberInList = 0;
  mt.Production->ActiveProgram->Name = "Demo Measurement Program";
  mt.Production->ActiveProgram->State->CurrentState->Value = {"en", "Running"};
  mt.Production->ActiveProgram->State->CurrentState->Number = 1;
  mt.Production->ActiveProgram->State->CurrentState->Id =
    UA_NODEID_NUMERIC(nsFromUri(m_pServer, constants::NsMachineToolUri), UA_MACHINETOOLID_PRODUCTIONSTATEMACHINETYPE_RUNNING);
}

void MitutoyoSim::InstantiateTools() {}

void MitutoyoSim::Simulate() {
  ++m_simStep;
  int i = m_simStep;
  if ((m_simStep % 2) == 1) {
    SimulateStacklight();
  }

  {
    mt.Production->ActiveProgram->State->CurrentState->Value = {"en", "Running"};
    mt.Production->ActiveProgram->State->CurrentState->Number = 1;
    mt.Production->ActiveProgram->State->CurrentState->Id =
      UA_NODEID_NUMERIC(nsFromUri(m_pServer, constants::NsMachineToolUri), UA_MACHINETOOLID_PRODUCTIONSTATEMACHINETYPE_RUNNING);
  }

  mt.Monitoring->MachineTool->PowerOnDuration = m_simStep / 3600;

  mt.Monitoring->MachineTool->FeedOverride->Value = 100.0;
  mt.Monitoring->MachineTool->FeedOverride->EURange->high = 100.0;
  mt.Monitoring->MachineTool->FeedOverride->EURange->low = 0.0;
  mt.Monitoring->MachineTool->FeedOverride->EngineeringUnits->DisplayName = {"", "%"};
}