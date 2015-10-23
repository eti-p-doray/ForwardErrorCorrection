/*******************************************************************************
 This file is part of FeCl.
 
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
 All rights reserved.
 
 FeCl is free software: you can redistribute it and/or modify
 it under the terms of the Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 FeCl is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the Lesser General Public License
 along with FeCl.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef FEC_LDPC_H
#define FEC_LDPC_H

#include <thread>
#include <random>
#include <chrono>
#include <algorithm>

#include <boost/serialization/export.hpp>

#include "../Codec.h"
#include "../BitMatrix.h"
#include "../Permutation.h"

namespace fec {
  
  /**
   *  This class represents an ldpc encode / decoder.
   *  It offers methods encode and to decode data given an Structure.
   *
   *  The structure of the parity bits generated by an Ldpc object is as follow.
   *
   *    | syst | parity |
   *
   *  where syst are the systematic msg bits and parity are the added parity required
   *  to create a consistent bloc.
   *
   *  The structure of the extrinsic information is the case
   *
   *    | check1 | check2 | ... |
   *
   *  where checkX are the messages at the last iteration from the check node X
   *  that would be transmitted to the connected bit nodes at the next iteration.
   */
  class Ldpc : public Codec
  {
    friend class boost::serialization::access;
  public:
    struct Gallager {
      static SparseBitMatrix matrix(size_t n, size_t wc, size_t wr, uint64_t seed = 0);
    };
    struct DvbS2 {
    public:
      static SparseBitMatrix matrix(size_t n, double rate);
      
    private:
      static const std::array<size_t, 2> length_;
      static const std::vector<std::vector<double>> rate_;
      static const std::vector<std::vector<size_t>> parameter_;
      static const std::vector<std::vector<std::vector<std::vector<size_t>>>> index_;
    };
    
    struct EncoderOptions
    {
      friend class Structure;
    public:
      EncoderOptions(const SparseBitMatrix& checkMatrix) {checkMatrix_ = checkMatrix;}
      
      SparseBitMatrix checkMatrix_;
    };
    
    struct DecoderOptions {
    public:
      DecoderOptions() = default;
      
      DecoderOptions& algorithm(Codec::DecoderAlgorithm algorithm) {algorithm_ = algorithm; return *this;}
      DecoderOptions& iterations(size_t n) {iterations_ = n; return *this;}
      DecoderOptions& gain(double gain) {gain_ = gain; return *this;}
      
      Codec::DecoderAlgorithm algorithm_ = Approximate;
      size_t iterations_;
      double gain_ = 1.0;
    };
    struct PunctureOptions {
    public:
      PunctureOptions() = default;
      
      PunctureOptions& mask(std::vector<bool> mask) {mask_ = mask; return *this;}
      PunctureOptions& systMask(std::vector<bool> mask) {systMask_ = mask; return *this;}
      
      std::vector<bool> systMask_;
      std::vector<bool> mask_;
    };
    struct Options : public EncoderOptions, DecoderOptions
    {
    public:
      Options(const SparseBitMatrix& checkMatrix) : EncoderOptions(checkMatrix) {}
    };
    /**
     *  This class represents a ldpc code structure.
     *  It provides a usefull interface to store and acces the structure information.
     */
    class Structure : public Codec::Structure {
      friend class ::boost::serialization::access;
    public:
      Structure() = default;
      Structure(const Options& options);
      Structure(const EncoderOptions&, const DecoderOptions&);
      Structure(const EncoderOptions&);
      virtual ~Structure() = default;
      
      virtual const char * get_key() const;
      
      void setDecoderOptions(const DecoderOptions& decoder);
      DecoderOptions getDecoderOptions() const;
      Permutation puncturing(const PunctureOptions& options) const;
      
      inline const SparseBitMatrix& checks() const {return H_;}
      inline size_t iterations() const {return iterations_;}
      
      void syndrome(std::vector<uint8_t>::const_iterator parity, std::vector<uint8_t>::iterator syndrome) const;
      virtual bool check(std::vector<BitField<size_t>>::const_iterator parity) const;
      virtual void encode(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity) const;
      
    protected:
      void setEncoderOptions(const EncoderOptions& encoder);
      
    private:
      template <typename Archive>
      void serialize(Archive & ar, const unsigned int version) {
        using namespace boost::serialization;
        ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Codec::Structure);
        ar & ::BOOST_SERIALIZATION_NVP(H_);
        ar & ::BOOST_SERIALIZATION_NVP(DC_);
        ar & ::BOOST_SERIALIZATION_NVP(T_);
        ar & ::BOOST_SERIALIZATION_NVP(A_);
        ar & ::BOOST_SERIALIZATION_NVP(B_);
        ar & ::BOOST_SERIALIZATION_NVP(iterations_);
      }
      
      void computeGeneratorMatrix(SparseBitMatrix H);
      
      SparseBitMatrix H_;
      SparseBitMatrix DC_;
      SparseBitMatrix T_;
      SparseBitMatrix A_;
      SparseBitMatrix B_;
      
      size_t iterations_;
    };
    
    Ldpc() = default;
    Ldpc(const Options& options, int workGroupSize = 8);
    Ldpc(const Structure& structure, int workGroupSize = 8);
    Ldpc(const EncoderOptions& encoder, const DecoderOptions& decoder, int workGroupSize = 8);
    Ldpc(const EncoderOptions& encoder, int workGroupSize = 8);
    Ldpc(const Ldpc& other) {*this = other;}
    virtual ~Ldpc() = default;
    Ldpc& operator=(const Ldpc& other) {Codec::operator=(other); structure_ = std::unique_ptr<Structure>(new Structure(other.structure())); return *this;}
    
    virtual const char * get_key() const;
    
    void setDecoderOptions(const DecoderOptions& decoder) {structure().setDecoderOptions(decoder);}
    DecoderOptions getDecoderOptions() const {return structure().getDecoderOptions();}
    
    Permutation puncturing(const PunctureOptions& options) {return structure().puncturing(options);}
    
  protected:
    Ldpc(std::unique_ptr<Structure>&& structure, int workGroupSize = 4) : Codec(std::move(structure), workGroupSize) {}
    
    inline const Structure& structure() const {return dynamic_cast<const Structure&>(Codec::structure());}
    inline Structure& structure() {return dynamic_cast<Structure&>(Codec::structure());}
    
    virtual void decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg, size_t n) const;
    virtual void soDecodeBlocks(InputIterator input, OutputIterator output, size_t n) const;
    
  private:
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version) {
      using namespace boost::serialization;
      ar.template register_type<Structure>();
      ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Codec);
    }
  };
  
}

BOOST_CLASS_EXPORT_KEY(fec::Ldpc);
BOOST_CLASS_TYPE_INFO(fec::Ldpc,extended_type_info_no_rtti<fec::Ldpc>);
BOOST_CLASS_EXPORT_KEY(fec::Ldpc::Structure);
BOOST_CLASS_TYPE_INFO(fec::Ldpc::Structure,extended_type_info_no_rtti<fec::Ldpc::Structure>);

#endif
