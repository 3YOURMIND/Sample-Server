#pragma once
#include "../TypeDefiniton.hpp"
#include "Constants.hpp"
#include "StackElement.hpp"
#include "../ns0/AnalogItem.hpp"
#include "../../src_generated/types_industrial_automation_generated.h"

namespace ia
{

  struct StackElementLight_t : public StackElement_t
  {
    BindableMemberValue<ns0::AnalogItem_t<float>> Intensity;
    BindableMemberValue<UA_SignalColor> SignalColor;
  };

} // namespace ia

REFL_TYPE(ia::StackElementLight_t,
          Bases<ia::StackElement_t>(),
          UmatiServerLib::attribute::UaObjectType{
              .NodeId = UmatiServerLib::constexp::NodeId(constants::NsIAUri, UA_INDUSTRIAL_AUTOMATIONID_STACKELEMENTLIGHTTYPE)})
REFL_FIELD(Intensity, UmatiServerLib::attribute::MemberInTypeNodeId{
               .NodeId = UmatiServerLib::constexp::NodeId(constants::NsIAUri, UA_INDUSTRIAL_AUTOMATIONID_STACKELEMENTLIGHTTYPE_INTENSITY)},
           UmatiServerLib::attribute::PlaceholderOptional())
REFL_FIELD(SignalColor, UmatiServerLib::attribute::MemberInTypeNodeId{
               .NodeId = UmatiServerLib::constexp::NodeId(constants::NsIAUri, UA_INDUSTRIAL_AUTOMATIONID_STACKELEMENTLIGHTTYPE_SIGNALCOLOR)},
           UmatiServerLib::attribute::PlaceholderOptional())
REFL_END