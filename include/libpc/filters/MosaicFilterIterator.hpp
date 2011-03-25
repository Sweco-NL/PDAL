/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/

#ifndef INCLUDED_FILTERS_MOSAICFILTERITERATOR_HPP
#define INCLUDED_FILTERS_MOSAICFILTERITERATOR_HPP

#include <libpc/libpc.hpp>

#include <vector>
//#include <libpc/Filter.hpp>
#include <libpc/FilterIterator.hpp>
//#include <libpc/Bounds.hpp>


namespace libpc { namespace filters {

class MosaicFilter;

class MosaicFilterSequentialIterator : public libpc::SequentialIterator
{
public:
    MosaicFilterSequentialIterator(const MosaicFilter& filter);
    ~MosaicFilterSequentialIterator();

private:
    boost::uint64_t skipImpl(boost::uint64_t);
    boost::uint32_t readImpl(PointBuffer&);
    bool atEndImpl() const;

    const std::vector<SequentialIterator*>& getPrevIterators() const;

    const MosaicFilter& m_filter;
    std::vector<SequentialIterator*> m_prevIterators;
};


} } // namespaces

#endif
