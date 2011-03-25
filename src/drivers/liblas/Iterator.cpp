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

#include <libpc/drivers/liblas/Iterator.hpp>

#include <liblas/factory.hpp>

#include <libpc/exceptions.hpp>
#include <libpc/PointBuffer.hpp>
#include <libpc/Utils.hpp>
#include <libpc/drivers/liblas/Reader.hpp>

namespace libpc { namespace drivers { namespace liblas {


//---------------------------------------------------------------------------
//
// LiblasIteratorBase
//
//---------------------------------------------------------------------------


LiblasIteratorBase::LiblasIteratorBase(const LiblasReader& reader)
    : m_reader(reader)
    , m_filename(reader.getFileName())
    , m_istream(NULL)
    , m_externalReader(NULL)
{
    m_istream = Utils::openFile(m_filename);

    ::liblas::ReaderFactory f;
    ::liblas::Reader extReader = f.CreateWithStream(*m_istream);
    m_externalReader = new ::liblas::Reader(extReader);

    return;
}


LiblasIteratorBase::~LiblasIteratorBase()
{
    delete m_externalReader;
    Utils::closeFile(m_istream);
}


::liblas::Reader& LiblasIteratorBase::getExternalReader() const
{
    return *m_externalReader;
}


const LiblasReader& LiblasIteratorBase::getReader() const
{
    return m_reader;
}


boost::uint32_t LiblasIteratorBase::readBuffer(PointBuffer& data)
{
    boost::uint32_t numPoints = data.getCapacity();
    boost::uint32_t i = 0;

    const Schema& schema = data.getSchema();

    const int indexX = schema.getDimensionIndex(Dimension::Field_X);
    const int indexY = schema.getDimensionIndex(Dimension::Field_Y);
    const int indexZ = schema.getDimensionIndex(Dimension::Field_Z);
    
    const int indexIntensity = schema.getDimensionIndex(Dimension::Field_Intensity);
    const int indexReturnNumber = schema.getDimensionIndex(Dimension::Field_ReturnNumber);
    const int indexNumberOfReturns = schema.getDimensionIndex(Dimension::Field_NumberOfReturns);
    const int indexScanDirectionFlag = schema.getDimensionIndex(Dimension::Field_ScanDirectionFlag);
    const int indexEdgeOfFlightLine = schema.getDimensionIndex(Dimension::Field_EdgeOfFlightLine);
    const int indexClassification = schema.getDimensionIndex(Dimension::Field_Classification);
    const int indexScanAngleRank = schema.getDimensionIndex(Dimension::Field_ScanAngleRank);
    const int indexUserData = schema.getDimensionIndex(Dimension::Field_UserData);
    const int indexPointSourceId = schema.getDimensionIndex(Dimension::Field_PointSourceId);
    
    const int indexTime = (getReader().hasTimeData() ? schema.getDimensionIndex(Dimension::Field_Time) : 0);

    const int indexRed = (getReader().hasColorData() ? schema.getDimensionIndex(Dimension::Field_Red) : 0);
    const int indexGreen = (getReader().hasColorData() ? schema.getDimensionIndex(Dimension::Field_Green) : 0);
    const int indexBlue = (getReader().hasColorData() ? schema.getDimensionIndex(Dimension::Field_Blue) : 0);

    //const int indexWavePacketDescriptorIndex = (m_hasWaveData ? schema.getDimensionIndex(Dimension::Field_WavePacketDescriptorIndex) : 0);
    //const int indexWaveformDataOffset = (m_hasWaveData ? schema.getDimensionIndex(Dimension::Field_WaveformDataOffset) : 0);
    //const int indexReturnPointWaveformLocation = (m_hasWaveData ? schema.getDimensionIndex(Dimension::Field_ReturnPointWaveformLocation) : 0);
    //const int indexWaveformXt = (m_hasWaveData ? schema.getDimensionIndex(Dimension::Field_WaveformXt) : 0);
    //const int indexWaveformYt = (m_hasWaveData ? schema.getDimensionIndex(Dimension::Field_WaveformYt) : 0);
    //const t indexWaveformZt = (m_hasWaveData ? schema.getDimensionIndex(Dimension::Field_WaveformZt) : 0);

    for (i=0; i<numPoints; i++)
    {
        bool ok = getExternalReader().ReadNextPoint();
        if (!ok)
        {
            throw libpc_error("liblas reader failed to retrieve point");
        }

        const ::liblas::Point& pt = getExternalReader().GetPoint();

        const boost::int32_t x = pt.GetRawX();
        const boost::int32_t y = pt.GetRawY();
        const boost::int32_t z = pt.GetRawZ();

        const boost::uint16_t intensity = pt.GetIntensity();
        const boost::int8_t returnNumber = (boost::int8_t)pt.GetReturnNumber();
        const boost::int8_t numberOfReturns = (boost::int8_t)pt.GetNumberOfReturns();
        const boost::int8_t scanDirFlag = (boost::int8_t)pt.GetScanDirection();
        const boost::int8_t edgeOfFlightLine = (boost::int8_t)pt.GetFlightLineEdge();
        const boost::uint8_t classification = pt.GetClassification().GetClass();
        const boost::int8_t scanAngleRank = pt.GetScanAngleRank();
        const boost::uint8_t userData = pt.GetUserData();
        const boost::uint16_t pointSourceId = pt.GetPointSourceID();
        
        data.setField(i, indexX, x);
        data.setField(i, indexY, y);
        data.setField(i, indexZ, z);

        data.setField(i, indexIntensity, intensity);
        data.setField(i, indexReturnNumber, returnNumber);
        data.setField(i, indexNumberOfReturns, numberOfReturns);
        data.setField(i, indexScanDirectionFlag, scanDirFlag);
        data.setField(i, indexEdgeOfFlightLine, edgeOfFlightLine);
        data.setField(i, indexClassification, classification);
        data.setField(i, indexScanAngleRank, scanAngleRank);
        data.setField(i, indexUserData, userData);
        data.setField(i, indexPointSourceId, pointSourceId);

        if (getReader().hasTimeData())
        {
            const double time = pt.GetTime();
            
            data.setField(i, indexTime, time);
        }

        if (getReader().hasColorData())
        {
            const ::liblas::Color color = pt.GetColor();
            const boost::uint16_t red = color.GetRed();
            const boost::uint16_t green = color.GetGreen();
            const boost::uint16_t blue = color.GetBlue();

            data.setField(i, indexRed, red);
            data.setField(i, indexGreen, green);
            data.setField(i, indexBlue, blue);
        }
        
        data.setNumPoints(i+1);
        if (getReader().hasWaveData())
        {
            throw not_yet_implemented("Waveform data (types 4 and 5) not supported");
        }
        
    }

    return numPoints;
}


//---------------------------------------------------------------------------
//
// SequentialIterator
//
//---------------------------------------------------------------------------

SequentialIterator::SequentialIterator(const LiblasReader& reader)
    : LiblasIteratorBase(reader)
    , libpc::SequentialIterator(reader)
{
    return;
}


SequentialIterator::~SequentialIterator()
{
    return;
}


boost::uint64_t SequentialIterator::skipImpl(boost::uint64_t count)
{
    boost::uint64_t newPos = getIndex() + count;

    size_t newPosX = (size_t)newPos;
    getExternalReader().Seek(newPosX);

    return count;
}



bool SequentialIterator::atEndImpl() const
{
    return getIndex() >= getStage().getNumPoints();
}


boost::uint32_t SequentialIterator::readImpl(PointBuffer& data)
{
    return readBuffer(data);
}


//---------------------------------------------------------------------------
//
// RandomIterator
//
//---------------------------------------------------------------------------

RandomIterator::RandomIterator(const LiblasReader& reader)
    : LiblasIteratorBase(reader)
    , libpc::RandomIterator(reader)
{
    return;
}


RandomIterator::~RandomIterator()
{
    return;
}


boost::uint64_t RandomIterator::seekImpl(boost::uint64_t pos)
{
    size_t posx = (size_t)pos; // BUG
    getExternalReader().Seek(posx);

    return pos;
}


boost::uint32_t RandomIterator::readImpl(PointBuffer& data)
{
    return readBuffer(data);
}


} } } // namespaces
