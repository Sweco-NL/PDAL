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

#ifndef INCLUDED_DRIVERS_LIBLAS_ITERATOR_HPP
#define INCLUDED_DRIVERS_LIBLAS_ITERATOR_HPP

#include <libpc/libpc.hpp>

//#include <libpc/Stage.hpp>
#include <libpc/Iterator.hpp>

#include <string>

//#include <libpc/drivers/liblas/Header.hpp>

// fwd decls
namespace liblas
{
    class Reader;
}

namespace libpc { namespace drivers { namespace liblas {

class LiblasReader;


class LiblasIteratorBase
{
public:
    LiblasIteratorBase(const LiblasReader& reader);
    ~LiblasIteratorBase();

protected:
    ::liblas::Reader& getExternalReader() const;
    const LiblasReader& getReader() const;
    
    boost::uint32_t readBuffer(PointBuffer& data);

private:
    const LiblasReader& m_reader;
    std::string m_filename;
    std::istream* m_istream;
    ::liblas::Reader* m_externalReader;
    
    LiblasIteratorBase& operator=(const LiblasIteratorBase&); // not implemented
    LiblasIteratorBase(const LiblasIteratorBase&); // not implemented};
};


class SequentialIterator : public LiblasIteratorBase, public libpc::SequentialIterator
{
public:
    SequentialIterator(const LiblasReader& reader);
    ~SequentialIterator();

private:
    boost::uint64_t skipImpl(boost::uint64_t count);
    boost::uint32_t readImpl(PointBuffer& data);
    bool atEndImpl() const;
};


class RandomIterator : public LiblasIteratorBase, public libpc::RandomIterator
{
public:
    RandomIterator(const LiblasReader& reader);
    ~RandomIterator();

private:
    boost::uint64_t seekImpl(boost::uint64_t pos);
    boost::uint32_t readImpl(PointBuffer& data);
};


} } } // namespaces

#endif
