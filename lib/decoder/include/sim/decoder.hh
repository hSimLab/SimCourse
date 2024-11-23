#ifndef __LIB_DECODER_INCLUDE_SIM_DECODER_HH__
#define __LIB_DECODER_INCLUDE_SIM_DECODER_HH__

#include "sim/isa.hh"

namespace sim::decoder {
isa::Instruction decode(isa::Word bytes);
}

#endif // __LIB_DECODER_INCLUDE_SIM_DECODER_HH__
