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

#ifndef INCLUDED_FILTERS_CROPFILTER_HPP
#define INCLUDED_FILTERS_CROPFILTER_HPP

#include <libpc/libpc.hpp>
//#include <libpc/export.hpp>
#include <libpc/Filter.hpp>
//#include <libpc/FilterIterator.hpp>
#include <libpc/Bounds.hpp>

namespace libpc
{
    class PointBuffer;
}

namespace libpc { namespace filters {

class CropFilterSequentialIterator;

// removes any points outside of the given range
// updates the header accordingly
class LIBPC_DLL CropFilter : public Filter
{
public:
    CropFilter(const Stage& prevStage, Bounds<double> const& bounds);

    const std::string& getName() const;

    bool supportsSequentialIterator() const { return true; }
    bool supportsRandomIterator() const { return false; }
    libpc::SequentialIterator* createSequentialIterator() const;
    libpc::RandomIterator* createRandomIterator() const { return NULL; }

    // returns number of points accepted into the data buffer (which may be less than data.getNumPoints(),
    // if we're calling this routine multiple times with the same buffer
    boost::uint32_t processBuffer(PointBuffer& dstData, const PointBuffer& srcData) const;

    const Bounds<double>& getBounds() const;

private:
    Bounds<double> m_bounds;

    CropFilter& operator=(const CropFilter&); // not implemented
    CropFilter(const CropFilter&); // not implemented
};


} } // namespaces

#endif
