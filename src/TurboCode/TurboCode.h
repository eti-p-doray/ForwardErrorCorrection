/*******************************************************************************
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Declaration of TurboCode class
 ******************************************************************************/

#ifndef TURBO_CODE_H
#define TURBO_CODE_H

#include <boost/serialization/export.hpp>

#include "../Code.h"
#include "../CodeStructure/TurboCodeStructure.h"
#include "../ConvolutionalCode/ConvolutionalCode.h"
#include "TurboCodeImpl.h"

namespace fec {

/**
 *  This class represents a turbo encode / decoder.
 *  It offers methods encode and to decode data given a TurboCodeStructure.
 *
 *  The structure of the parity bits generated by a TurboCode object is as follow
 *
 *    | syst | systTail | convOutput1 | tailOutpu1 | convOutput2 | tailOutpu2 | ... |
 *
 *  where syst are the systematic bits, systTail are the tail bit 
 *  added to the msg for termination of the constituents 1, 2, ..., respectively,
 *  convOutputX and tailOutputX are the output parity of the  msg and the tail 
 *  generated by the constituent convolutional code X.
 *
 *  The structure of the extrinsic information is the case of serial decoding
 *
 *    | msg | systTail |
 *
 *  where msg are the extrinsic msg L-values generated by the last constituent
 *  code involved in each msg bit.
 *
 *  And in the case of parallel decoding
 *
 *    | msg1 | systTail1 | msg2 | systTail2 | ... |
 *
 *  where msgX are the extrinsic msg L-values generated by the constituent code X
 *  and systTailX are the tail bit added to the msg 
 *  for termination of the constituent X.
 */
class TurboCode : public Code
{
  friend class boost::serialization::access;
public:
  TurboCode(const TurboCodeStructure& codeStructure, int workGroupdSize = 4);
  virtual ~TurboCode() = default;
  
  virtual const char * get_key() const;
  
  virtual size_t msgSize() const {return codeStructure_.msgSize();}
  virtual size_t paritySize() const {return codeStructure_.paritySize();}
  virtual size_t extrinsicSize() const {
    size_t extrinsicSize = 0;
    switch (codeStructure_.schedulingType()) {
      default:
      case TurboCodeStructure::Serial:
        return codeStructure_.msgSize() + codeStructure_.msgTailSize();
        break;
        
      case TurboCodeStructure::Parallel:
        for (auto & i : codeStructure_.constituents()) {
          extrinsicSize += i.msgSize() + i.msgTailSize();
        }
        return extrinsicSize;
        break;
    }
  }
  virtual const CodeStructure& structure() const {return codeStructure_;}
  
  inline size_t getIterationCount() const {return codeStructure_.iterationCount();}
  inline void setIterationCount(size_t count) {codeStructure_.setIterationCount(count);}
  
protected:
  TurboCode() = default;
  
  virtual void encodeBloc(std::vector<uint8_t>::const_iterator messageIt, std::vector<uint8_t>::iterator parityIt) const;
  
  virtual void appDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::const_iterator extrinsicIn, std::vector<LlrType>::iterator messageOut, std::vector<LlrType>::iterator extrinsicOut, size_t n) const;
  virtual void softOutDecodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<LlrType>::iterator messageOut, size_t n) const;
  virtual void decodeNBloc(std::vector<LlrType>::const_iterator parityIn, std::vector<uint8_t>::iterator messageOut, size_t n) const;
  
  template <typename T>
  void pack(typename std::vector<T>::const_iterator parityIn, typename std::vector<T>::iterator parityOut);
  
  template <typename T>
  void unPack(typename std::vector<T>::const_iterator parityIn, typename std::vector<T>::iterator parityOut);

private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    using namespace boost::serialization;
    ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Code);
    ar & ::BOOST_SERIALIZATION_NVP(codeStructure_);
  }
  
  TurboCodeStructure codeStructure_;
};
  
}

BOOST_CLASS_EXPORT_KEY(fec::TurboCode);
BOOST_CLASS_TYPE_INFO(fec::TurboCode,extended_type_info_no_rtti<fec::TurboCode>);

template <typename T>
void fec::TurboCode::pack(typename std::vector<T>::const_iterator parityIn, typename std::vector<T>::iterator parityOut)
{
  auto parityOutIt = parityOut;
  auto msgInIt = parityIn;
  auto parityInIt = parityIn + codeStructure_.msgSize() + codeStructure_.msgTailSize();
  for (size_t i = 0; i < codeStructure_.msgSize(); ++i) {
    *parityOut = *msgInIt;
    ++msgInIt;
    ++parityOut;
    for (size_t j = 0; j < codeStructure_.constituentCount(); ++j) {
      if (i < codeStructure_.constituent(i).blocSize()) {
        for (size_t k = 0; k < codeStructure_.constituent(i).trellis().outputSize(); ++k) {
          *parityOutIt = *parityInIt;
          ++parityOutIt;
        }
      }
    }
  }
  parityInIt = parityIn + codeStructure_.msgSize() + codeStructure_.msgTailSize();
  for (size_t i = 0; i < codeStructure_.constituentCount(); ++i) {
    parityInIt += codeStructure_.constituent(i).paritySize() - codeStructure_.constituent(i).tailSize() * codeStructure_.constituent(i).trellis().outputSize();
    for (size_t j = 0; j < codeStructure_.constituent(i).tailSize(); ++j) {
      for (size_t k = 0; k < codeStructure_.constituent(i).trellis().inputSize(); ++k) {
        *parityOutIt = *msgInIt;
        ++parityOutIt;
        ++msgInIt;
      }
      for (size_t k = 0; k < codeStructure_.constituent(i).trellis().outputSize(); ++k) {
        *parityOutIt = *parityInIt;
        ++parityOutIt;
        ++parityInIt;
      }
    }
  }
}

template <typename T>
void fec::TurboCode::unPack(typename std::vector<T>::const_iterator parityIn, typename std::vector<T>::iterator parityOut)
{
  
}


#endif
