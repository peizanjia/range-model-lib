#include <rbl_error.h>
#include <rbl_logger.h>
#include <rbl_utils.h>

#include <cmath>
#include <utility>
#include <set>

#include "rml_file_io.h"
#include "rml_problem_setup.h"


void RFileIO::writeNewLineAscii(RSaveFile &outFile)
{
    outFile.getTextStream() << RConstants::endl;
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write the new line.");
    }
} /* RFileIO::writeNewLineAscii */


void RFileIO::writeNewLineAscii(RFile &outFile)
{
    outFile.getTextStream() << RConstants::endl;
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write the new line.");
    }
} /* RFileIO::writeNewLineAscii */


/*********************************************************************
 *  bool                                                             *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, bool &bValue)
{
    QTextStream &stream = inFile.getTextStream();
    stream.skipWhiteSpace();
    QChar value;
    stream >> value;
    bValue = (value.unicode() == 1 || value == QLatin1Char('1'));
    if (stream.status() != QTextStream::Ok)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF, "Failed to read bool value.");
    }
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, bool &bValue)
{
    inFile.read((char*)&bValue,sizeof(bool));
    if (inFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,"Failed to read bool value.");
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const bool &bValue, bool addNewLine)
{
    if (!addNewLine)
    {
        outFile.getTextStream() << bValue;
    }
    else
    {
        outFile.getTextStream() << char(bValue) << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write bool value.");
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const bool &bValue)
{
    outFile.write((char*)&bValue,sizeof(bool));
    if (outFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write bool value.");
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  char                                                             *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, char &cValue)
{
    inFile.getTextStream() >> cValue;
    if (inFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF, "Failed to read char value.");
    }
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, char &cValue)
{

    inFile.read((char*)&cValue,sizeof(char));
    if (inFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,"Failed to read char value.");
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const char &cValue, bool addNewLine)
{
    if (!addNewLine)
    {
        outFile.getTextStream() << cValue;
    }
    else
    {
        outFile.getTextStream() << cValue << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write char value.");
    }
} /* RFileIO::writeAscii */


void RFileIO::writeAscii(RFile &outFile, const char &cValue, bool addNewLine)
{
    if (!addNewLine)
    {
        outFile.getTextStream() << cValue;
    }
    else
    {
        outFile.getTextStream() << cValue << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write char value.");
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const char &cValue)
{
    outFile.write((char*)&cValue,sizeof(char));
    if (outFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write char value.");
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  int                                                              *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, int &iValue)
{
    inFile.getTextStream() >> iValue;
    if (inFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF, "Failed to read int value.");
    }
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, int &iValue)
{
    inFile.read((char*)&iValue,sizeof(int));
    if (inFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,"Failed to read int value.");
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const int &iValue, bool addNewLine)
{
    if (!addNewLine)
    {
        outFile.getTextStream() << iValue;
    }
    else
    {
        outFile.getTextStream() << iValue << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write int value.");
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const int &iValue)
{
    outFile.write((char*)&iValue,sizeof(int));
    if (outFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write int value.");
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  unsigned int                                                     *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, unsigned int &uValue)
{
    inFile.getTextStream() >> uValue;
    if (inFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF, "Failed to read unsigned int value.");
    }
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, unsigned int &uValue)
{
    inFile.read((char*)&uValue,sizeof(unsigned int));
    if (inFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,"Failed to read unsigned int value.");
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const unsigned int &uValue, bool addNewLine)
{
    if (!addNewLine)
    {
        outFile.getTextStream() << uValue;
    }
    else
    {
        outFile.getTextStream() << uValue << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write unsigned int value.");
    }
} /* RFileIO::writeAscii */


void RFileIO::writeAscii(RFile &outFile, const unsigned int &uValue, bool addNewLine)
{
    if (!addNewLine)
    {
        outFile.getTextStream() << uValue;
    }
    else
    {
        outFile.getTextStream() << uValue << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write unsigned int value.");
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const unsigned int &uValue)
{
    outFile.write((char*)&uValue,sizeof(unsigned int));
    if (outFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write unsigned int value.");
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  double                                                           *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, double &dValue)
{
    inFile.getTextStream() >> dValue;
    if (inFile.getTextStream().status() != QTextStream::Ok)
    {
        if (inFile.getTextStream().status() == QTextStream::ReadCorruptData)
        {
            inFile.getTextStream().resetStatus();
            return;
        }
        throw RError(RError::Type::ReadFile,R_ERROR_REF, "Failed to read double value.");
    }
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, double &dValue)
{
    inFile.read((char*)&dValue,sizeof(double));
    if (inFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,"Failed to read double value.");
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const double &dValue, bool addNewLine)
{
    // QTextStream formats negative zero as zero. Native source/restart identity
    // preserves binary64 signs, so retain this bit in the versioned model text.
    if (outFile.getVersion() > RVersion(1,3,19) && dValue == 0.0 && std::signbit(dValue))
    {
        outFile.getTextStream() << "-0";
        if (addNewLine) outFile.getTextStream() << RConstants::endl;
    }
    else if (!addNewLine)
    {
        outFile.getTextStream() << dValue;
    }
    else
    {
        outFile.getTextStream() << dValue << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write double value.");
    }
} /* RFileIO::writeAscii */


void RFileIO::writeAscii(RFile &outFile, const double &dValue, bool addNewLine)
{
    if (!addNewLine)
    {
        outFile.getTextStream() << dValue;
    }
    else
    {
        outFile.getTextStream() << dValue << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write double value.");
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const double &dValue)
{
    outFile.write((char*)&dValue,sizeof(double));
    if (outFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write double value.");
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  qsizetype                                                           *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, qsizetype &sValue)
{
    inFile.getTextStream() >> sValue;
    if (inFile.getTextStream().status() != QTextStream::Ok)
    {
        if (inFile.getTextStream().status() == QTextStream::ReadCorruptData)
        {
            inFile.getTextStream().resetStatus();
            return;
        }
        throw RError(RError::Type::ReadFile,R_ERROR_REF, "Failed to read qsizetype value.");
    }
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, qsizetype &sValue)
{
    inFile.read((char*)&sValue,sizeof(qsizetype));
    if (inFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,"Failed to read qsizetype value.");
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const qsizetype &sValue, bool addNewLine)
{
    if (!addNewLine)
    {
        outFile.getTextStream() << sValue;
    }
    else
    {
        outFile.getTextStream() << sValue << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write qsizetype value.");
    }
} /* RFileIO::writeAscii */


void RFileIO::writeAscii(RFile &outFile, const qsizetype &sValue, bool addNewLine)
{
    if (!addNewLine)
    {
        outFile.getTextStream() << sValue;
    }
    else
    {
        outFile.getTextStream() << sValue << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write qsizetype value.");
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const qsizetype &sValue)
{
    outFile.write((char*)&sValue,sizeof(qsizetype));
    if (outFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write qsizetype value.");
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  QUuid                                                            *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, QUuid &uValue)
{
    QString sValue;
    RFileIO::readAscii(inFile,sValue);
    uValue = QUuid(sValue);
} /* RFileIO::readAscii */

void RFileIO::readBinary(RFile &inFile, QUuid &uValue)
{
    QString sValue;
    RFileIO::readBinary(inFile,sValue);
    uValue = QUuid(sValue);
} /* RFileIO::readBinary */

void RFileIO::writeAscii(RSaveFile &outFile, const QUuid &uValue, bool addNewLine)
{
    RFileIO::writeAscii(outFile,uValue.toString(QUuid::WithoutBraces),addNewLine);
} /* RFileIO::writeAscii */

void RFileIO::writeAscii(RFile &outFile, const QUuid &uValue, bool addNewLine)
{
    RFileIO::writeAscii(outFile,uValue.toString(QUuid::WithoutBraces),addNewLine);
} /* RFileIO::writeAscii */

void RFileIO::writeBinary(RSaveFile &outFile, const QUuid &uValue)
{
    RFileIO::writeBinary(outFile,uValue.toString(QUuid::WithoutBraces));
} /* RFileIO::writeBinary */


/*********************************************************************
 *  QString                                                          *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, QString &sValue)
{
    sValue.clear();
    QChar endChar = '\0';

    bool sStarted = false;
    bool cStarted = false;
    bool readNext = true;
    while (readNext)
    {
        QChar c;
        inFile.getTextStream() >> c;
        if (inFile.getTextStream().status() != QTextStream::Ok)
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF, "Failed to read string value.");
        }
        if (c == '"')
        {
            if (cStarted)
            {
                readNext = false;
            }
            cStarted = true;
        }
        else if (c.isSpace())
        {
            if (!cStarted && sStarted)
            {
                readNext = false;
            }
            if (cStarted && sStarted)
            {
                sValue += c;
                sStarted = true;
            }
        }
        else
        {
            sValue += c;
            sStarted = true;
        }
    }

    if (!sValue.isEmpty())
    {
        if (sValue[sValue.length()-1] == endChar && sValue.length() > 1)
        {
            sValue.remove(sValue.length()-1,1);
        }
        if (sValue[0] == endChar)
        {
            sValue.remove(0,1);
        }
    }

//    inFile.setCodec(textCodec);
//    inFile.setAutoDetectUnicode(autoDetectUnicode);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, QString &sValue)
{
    int stringSize = 0;
    RFileIO::readBinary(inFile,stringSize);
    QByteArray array = inFile.read(stringSize);
    if (inFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF, "Failed to read string value.");
    }
    sValue.clear();
    sValue.append(array);
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const QString &sValue, bool addNewLine)
{
//    QTextCodec *textCodec = outFile.codec();
//    outFile.setCodec("UTF-8");
    QString oValue(sValue);
    if (oValue.isEmpty())
    {
        oValue = "\"\"";
    }
    if (!addNewLine)
    {
        outFile.getTextStream() << oValue;
    }
    else
    {
        outFile.getTextStream() << oValue << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF, "Failed to write string value.");
    }
//    outFile.setCodec(textCodec);
} /* RFileIO::writeAscii */


void RFileIO::writeAscii(RFile &outFile, const QString &sValue, bool addNewLine)
{
//    QTextCodec *textCodec = outFile.codec();
//    outFile.setCodec("UTF-8");
    QString oValue(sValue);
    if (oValue.isEmpty())
    {
        oValue = "\"\"";
    }
    if (!addNewLine)
    {
        outFile.getTextStream() << oValue;
    }
    else
    {
        outFile.getTextStream() << oValue << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF, "Failed to write string value.");
    }
//    outFile.setCodec(textCodec);
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const QString &sValue)
{
    QByteArray array = sValue.toUtf8();
//    RFileIO::writeBinary(outFile,array.size());
//    outFile.write(array,array.size());
    RFileIO::writeBinary(outFile,uint(strlen(array.constData())));
    outFile.write(array.constData(),uint(strlen(array.constData())));
    if (outFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF, "Failed to write string value.");
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  std::string                                                      *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, std::string &sValue)
{
    QString qString;
    RFileIO::readAscii(inFile,qString);
    sValue = qString.toStdString();
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, std::string &sValue)
{
    QString qString;
    RFileIO::readBinary(inFile,qString);
    sValue = qString.toStdString();
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const char *sValue, bool addNewLine)
{
    RFileIO::writeAscii(outFile,QString(sValue),addNewLine);
} /* RFileIO::writeAscii */


void RFileIO::writeAscii(RSaveFile &outFile, const std::string &sValue, bool addNewLine)
{
    RFileIO::writeAscii(outFile,QString(sValue.c_str()),addNewLine);
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const char *sValue)
{
    RFileIO::writeBinary(outFile,QString(sValue));
} /* RFileIO::writeBinary */


void RFileIO::writeBinary(RSaveFile &outFile, const std::string &sValue)
{
    RFileIO::writeBinary(outFile,QString(sValue.c_str()));
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RIVector                                                         *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RIVector &iVector, bool readSize)
{
    unsigned int nr = 0;

    if (readSize)
    {
        RFileIO::readAscii(inFile,nr);
        iVector.resize(nr);
    }
    else
    {
        nr = iVector.getNRows();
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::readAscii(inFile,iVector[i]);
    }
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RIVector &iVector, bool readSize)
{
    unsigned int nr = 0;

    if (readSize)
    {
        RFileIO::readBinary(inFile,nr);
        iVector.resize(nr);
    }
    else
    {
        nr = iVector.getNRows();
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::readBinary(inFile,iVector[i]);
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RIVector &iVector, bool writeSize, bool addNewLine)
{
    unsigned int nr = iVector.getNRows();
    if (writeSize)
    {
        if (addNewLine)
        {
            RFileIO::writeAscii(outFile,nr,true);
        }
        else
        {
            RFileIO::writeAscii(outFile,nr,false);
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::writeAscii(outFile,iVector[i], false);
        if (i+1 < nr)
        {
            RFileIO::writeAscii(outFile,' ', false);
        }
    }
    if (addNewLine)
    {
        RFileIO::writeNewLineAscii(outFile);
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RIVector &iVector, bool writeSize)
{
    unsigned int nr = iVector.getNRows();
    if (writeSize)
    {
        RFileIO::writeBinary(outFile,nr);
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::writeBinary(outFile,iVector[i]);
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RUVector                                                         *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RUVector &uVector, bool readSize)
{
    unsigned int nr = 0;

    if (readSize)
    {
        RFileIO::readAscii(inFile,nr);
        uVector.resize(nr);
    }
    else
    {
        nr = uVector.getNRows();
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::readAscii(inFile,uVector[i]);
    }
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RUVector &uVector, bool readSize)
{
    unsigned int nr = 0;

    if (readSize)
    {
        RFileIO::readBinary(inFile,nr);
        uVector.resize(nr);
    }
    else
    {
        nr = uVector.getNRows();
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::readBinary(inFile,uVector[i]);
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RUVector &uVector, bool writeSize, bool addNewLine)
{
    unsigned int nr = uVector.getNRows();
    if (writeSize)
    {
        if (addNewLine)
        {
            RFileIO::writeAscii(outFile,nr,true);
        }
        else
        {
            RFileIO::writeAscii(outFile,nr,false);
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::writeAscii(outFile,uVector[i], false);
        if (i+1 < nr)
        {
            RFileIO::writeAscii(outFile,' ', false);
        }
    }
    if (addNewLine)
    {
        RFileIO::writeNewLineAscii(outFile);
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RUVector &uVector, bool writeSize)
{
    unsigned int nr = uVector.getNRows();
    if (writeSize)
    {
        RFileIO::writeBinary(outFile,nr);
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::writeBinary(outFile,uVector[i]);
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RRVector                                                         *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RRVector &rVector, bool readSize)
{
    unsigned int nr = 0;

    if (readSize)
    {
        RFileIO::readAscii(inFile,nr);
        rVector.resize(nr);
    }
    else
    {
        nr = rVector.getNRows();
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::readAscii(inFile,rVector[i]);
    }
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RRVector &rVector, bool readSize)
{
    unsigned int nr = 0;

    if (readSize)
    {
        RFileIO::readBinary(inFile,nr);
        rVector.resize(nr);
    }
    else
    {
        nr = rVector.getNRows();
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::readBinary(inFile,rVector[i]);
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RRVector &rVector, bool writeSize, bool addNewLine)
{
    unsigned int nr = rVector.getNRows();
    if (writeSize)
    {
        RFileIO::writeAscii(outFile,nr,addNewLine);
        if (!addNewLine)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::writeAscii(outFile,rVector[i], false);
        if (i+1 < nr)
        {
            RFileIO::writeAscii(outFile,' ', false);
        }
    }
    if (addNewLine)
    {
        RFileIO::writeNewLineAscii(outFile);
    }
} /* RFileIO::writeAscii */


void RFileIO::writeAscii(RFile &outFile, const RRVector &rVector, bool writeSize, bool addNewLine)
{
    unsigned int nr = rVector.getNRows();
    if (writeSize)
    {
        RFileIO::writeAscii(outFile,nr,addNewLine);
        if (!addNewLine)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::writeAscii(outFile,rVector[i], false);
        if (i+1 < nr)
        {
            RFileIO::writeAscii(outFile,' ', false);
        }
    }
    if (addNewLine)
    {
        RFileIO::writeNewLineAscii(outFile);
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RRVector &rVector, bool writeSize)
{
    unsigned int nr = rVector.getNRows();
    if (writeSize)
    {
        RFileIO::writeBinary(outFile,nr);
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::writeBinary(outFile,rVector[i]);
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RR3Vector                                                         *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RR3Vector &rVector)
{
    RFileIO::readAscii(inFile,rVector[0]);
    RFileIO::readAscii(inFile,rVector[1]);
    RFileIO::readAscii(inFile,rVector[2]);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RR3Vector &rVector)
{
    RFileIO::readBinary(inFile,rVector[0]);
    RFileIO::readBinary(inFile,rVector[1]);
    RFileIO::readBinary(inFile,rVector[2]);
} /* RFileIO::readAscii */


void RFileIO::writeAscii(RSaveFile &outFile, const RR3Vector &rVector, bool addNewLine)
{
    RFileIO::writeAscii(outFile,rVector[0],addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile," ",false);
    }
    RFileIO::writeAscii(outFile,rVector[1],addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile," ",false);
    }
    RFileIO::writeAscii(outFile,rVector[2],addNewLine);
} /* RFileIO::readAscii */


void RFileIO::writeAscii(RFile &outFile, const RR3Vector &rVector, bool addNewLine)
{
    RFileIO::writeAscii(outFile,rVector[0],addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile," ",false);
    }
    RFileIO::writeAscii(outFile,rVector[1],addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile," ",false);
    }
    RFileIO::writeAscii(outFile,rVector[2],addNewLine);
} /* RFileIO::readAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RR3Vector &rVector)
{
    RFileIO::writeBinary(outFile,rVector[0]);
    RFileIO::writeBinary(outFile,rVector[1]);
    RFileIO::writeBinary(outFile,rVector[2]);
} /* RFileIO::readAscii */


/*********************************************************************
 *  RIMatrix                                                         *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RIMatrix &iMatrix, bool readSize)
{
    unsigned int nr = 0;

    if (readSize)
    {
        unsigned int nc = 0;
        RFileIO::readAscii(inFile,nr);
        RFileIO::readAscii(inFile,nc);
        iMatrix.resize(nr,nc);
    }
    else
    {
        nr = iMatrix.getNRows();
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::readAscii(inFile,iMatrix[i],false);
    }
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RIMatrix &iMatrix, bool readSize)
{
    unsigned int nr = 0;

    if (readSize)
    {
        unsigned int nc = 0;
        RFileIO::readBinary(inFile,nr);
        RFileIO::readBinary(inFile,nc);
        iMatrix.resize(nr,nc);
    }
    else
    {
        nr = iMatrix.getNRows();
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::readBinary(inFile,iMatrix[i]);
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RIMatrix &iMatrix, bool writeSize, bool addNewLine)
{
    unsigned int nr = iMatrix.getNRows();
    if (writeSize)
    {
        unsigned int nc = iMatrix.getNColumns();
        RFileIO::writeAscii(outFile,nr,false);
        RFileIO::writeAscii(outFile,' ',false);
        if (addNewLine)
        {
            RFileIO::writeAscii(outFile,nc,true);
        }
        else
        {
            RFileIO::writeAscii(outFile,nc,false);
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::writeAscii(outFile,iMatrix[i],writeSize,addNewLine);
        if (i+1 < nr && !addNewLine)
        {
            RFileIO::writeAscii(outFile,' ');
        }
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RIMatrix &iMatrix, bool writeSize)
{
    unsigned int nr = iMatrix.getNRows();
    if (writeSize)
    {
        RFileIO::writeBinary(outFile,nr);
        RFileIO::writeBinary(outFile,iMatrix.getNColumns());
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::writeBinary(outFile,iMatrix[i],writeSize);
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RRMatrix                                                         *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RRMatrix &rMatrix, bool readSize)
{

    unsigned int nr = 0;

    if (readSize)
    {
        unsigned int nc = 0;
        RFileIO::readAscii(inFile,nr);
        RFileIO::readAscii(inFile,nc);
        rMatrix.resize(nr,nc);
    }
    else
    {
        nr = rMatrix.getNRows();
    }
    for (unsigned int i=0;i<nr;i++)
    {
        try {
        RFileIO::readAscii(inFile,rMatrix[i],false);
        } catch (const RError &error)
        {
            RLogger::warning("i = %u\n",i);
            throw error;
        }
    }
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RRMatrix &rMatrix, bool readSize)
{
    unsigned int nr = 0;

    if (readSize)
    {
        unsigned int nc = 0;
        RFileIO::readBinary(inFile,nr);
        RFileIO::readBinary(inFile,nc);
        rMatrix.resize(nr,nc);
    }
    else
    {
        nr = rMatrix.getNRows();
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::readBinary(inFile,rMatrix[i],false);
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RRMatrix &rMatrix, bool writeSize, bool addNewLine)
{
    unsigned int nr = rMatrix.getNRows();
    if (writeSize)
    {
        unsigned int nc = rMatrix.getNColumns();
        RFileIO::writeAscii(outFile,nr,false);
        RFileIO::writeAscii(outFile,' ',false);
        if (addNewLine)
        {
            RFileIO::writeAscii(outFile,nc,true);
        }
        else
        {
            RFileIO::writeAscii(outFile,nc,false);
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::writeAscii(outFile,rMatrix[i],false,addNewLine);
        if (i+1 < nr && !addNewLine)
        {
            RFileIO::writeAscii(outFile,' ');
        }
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile  &outFile, const RRMatrix &rMatrix, bool writeSize)
{
    unsigned int nr = rMatrix.getNRows();
    if (writeSize)
    {
        RFileIO::writeBinary(outFile,nr);
        RFileIO::writeBinary(outFile,rMatrix.getNColumns());
    }
    for (unsigned int i=0;i<nr;i++)
    {
        RFileIO::writeBinary(outFile,rMatrix[i],false);
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RVersion                                                         *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RVersion &version)
{
    QString versionStr;
    RFileIO::readAscii(inFile,versionStr);
    version = RVersion(versionStr);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RVersion &version)
{
    unsigned int major = 0;
    unsigned int minor = 0;
    unsigned int release = 0;
    unsigned int padding;
    RFileIO::readBinary(inFile,major);
    RFileIO::readBinary(inFile,minor);
    RFileIO::readBinary(inFile,release);
    RFileIO::readBinary(inFile,padding);
    version = RVersion(major,minor,release);
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RVersion &version, bool addNewLine)
{
    RFileIO::writeAscii(outFile,version.toString(),addNewLine);
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile  &outFile, const RVersion &version)
{
    RFileIO::writeBinary(outFile,version.getMajor());
    RFileIO::writeBinary(outFile,version.getMinor());
    RFileIO::writeBinary(outFile,version.getRelease());
    RFileIO::writeBinary(outFile,0);
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RValueVector                                                     *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RValueVector &valueVector)
{
    unsigned int n;
    QString name;
    QString units;

    RFileIO::readAscii(inFile,name);
    RFileIO::readAscii(inFile,units);
    RFileIO::readAscii(inFile,n);

    valueVector.setName(name);
    valueVector.setUnits(units);
    valueVector.resize(n);

    for (unsigned int i=0;i<n;i++)
    {
        RFileIO::readAscii(inFile,valueVector[i]);
    }
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RValueVector &valueVector)
{
    unsigned int n;
    QString name;
    QString units;

    RFileIO::readBinary(inFile,name);
    RFileIO::readBinary(inFile,units);
    RFileIO::readBinary(inFile,n);

    valueVector.setName(name);
    valueVector.setUnits(units);
    valueVector.resize(n);

    for (unsigned int i=0;i<n;i++)
    {
        RFileIO::readBinary(inFile,valueVector[i]);
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RValueVector &valueVector, bool addNewLine)
{
    unsigned int n = valueVector.size();

    RFileIO::writeAscii(outFile,"\"" + valueVector.getName() + "\"",addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile, "\"" + valueVector.getUnits() + "\"",addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,n,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }

    for (unsigned int i=0;i<n;i++)
    {
        RFileIO::writeAscii(outFile,valueVector[i],addNewLine);
        if (!addNewLine && i+1<n)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RValueVector &valueVector)
{
    unsigned int n = valueVector.size();

    RFileIO::writeBinary(outFile,valueVector.getName());
    RFileIO::writeBinary(outFile,valueVector.getUnits());
    RFileIO::writeBinary(outFile,n);

    for (unsigned int i=0;i<n;i++)
    {
        RFileIO::writeBinary(outFile,valueVector[i]);
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RPlane                                                           *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RPlane &plane)
{
    RR3Vector position;
    RR3Vector normal;

    RFileIO::readAscii(inFile,position);
    RFileIO::readAscii(inFile,normal);

    plane.setPosition(position);
    plane.setNormal(normal);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RPlane &plane)
{
    RR3Vector position;
    RR3Vector normal;

    RFileIO::readBinary(inFile,position);
    RFileIO::readBinary(inFile,normal);

    plane.setPosition(position);
    plane.setNormal(normal);
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RPlane &plane, bool addNewLine)
{
    RFileIO::writeAscii(outFile,plane.getPosition(),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,plane.getNormal(),addNewLine);
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RPlane &plane)
{
    RFileIO::writeBinary(outFile,plane.getPosition());
    RFileIO::writeBinary(outFile,plane.getNormal());
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RValueTable                                                      *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RValueTable &valueTable)
{
    unsigned int n;
    QString keyName;
    QString keyUnits;
    QString valueName;
    QString valueUnits;
    double key;
    double value;

    RFileIO::readAscii(inFile,keyName);
    RFileIO::readAscii(inFile,keyUnits);
    RFileIO::readAscii(inFile,valueName);
    RFileIO::readAscii(inFile,valueUnits);
    RFileIO::readAscii(inFile,n);

    valueTable.clear();

    valueTable.setKeyName(keyName);
    valueTable.setKeyUnits(keyUnits);
    valueTable.setValueName(valueName);
    valueTable.setValueUnits(valueUnits);

    for (unsigned int i=0;i<n;i++)
    {
        RFileIO::readAscii(inFile,key);
        RFileIO::readAscii(inFile,value);
        valueTable.add(key,value);
    }
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RValueTable &valueTable)
{
    unsigned int n;
    QString keyName;
    QString keyUnits;
    QString valueName;
    QString valueUnits;
    double key;
    double value;

    RFileIO::readBinary(inFile,keyName);
    RFileIO::readBinary(inFile,keyUnits);
    RFileIO::readBinary(inFile,valueName);
    RFileIO::readBinary(inFile,valueUnits);
    RFileIO::readBinary(inFile,n);

    valueTable.clear();

    valueTable.setKeyName(keyName);
    valueTable.setKeyUnits(keyUnits);
    valueTable.setValueName(valueName);
    valueTable.setValueUnits(valueUnits);

    for (unsigned int i=0;i<n;i++)
    {
        RFileIO::readBinary(inFile,key);
        RFileIO::readBinary(inFile,value);
        valueTable.add(key,value);
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RValueTable &valueTable, bool addNewLine)
{
    unsigned int n = valueTable.size();

    RFileIO::writeAscii(outFile,"\"" + valueTable.getKeyName() + "\"",addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,"\"" + valueTable.getKeyUnits() + "\"",addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,"\"" + valueTable.getValueName() + "\"",addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,"\"" + valueTable.getValueUnits() + "\"",addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,n,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }

    for (unsigned int i=0;i<n;i++)
    {
        RFileIO::writeAscii(outFile,valueTable.getKey(i),addNewLine);
        if (!addNewLine)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
        RFileIO::writeAscii(outFile,valueTable.getValue(i),addNewLine);
        if (!addNewLine && i+1 < n)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RValueTable &valueTable)
{
    unsigned int n = valueTable.size();

    RFileIO::writeBinary(outFile,valueTable.getKeyName());
    RFileIO::writeBinary(outFile,valueTable.getKeyUnits());
    RFileIO::writeBinary(outFile,valueTable.getValueName());
    RFileIO::writeBinary(outFile,valueTable.getValueUnits());
    RFileIO::writeBinary(outFile,n);

    for (unsigned int i=0;i<n;i++)
    {
        RFileIO::writeBinary(outFile,valueTable.getKey(i));
        RFileIO::writeBinary(outFile,valueTable.getValue(i));
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  QColor                                                           *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, QColor &color)
{
    int r,g,b,a;
    RFileIO::readAscii(inFile,r);
    RFileIO::readAscii(inFile,g);
    RFileIO::readAscii(inFile,b);
    RFileIO::readAscii(inFile,a);
    color = QColor(r,g,b,a);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, QColor &color)
{
    int r,g,b,a;
    RFileIO::readBinary(inFile,r);
    RFileIO::readBinary(inFile,g);
    RFileIO::readBinary(inFile,b);
    RFileIO::readBinary(inFile,a);
    color = QColor(r,g,b,a);
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const QColor &color, bool addNewLine)
{
    RFileIO::writeAscii(outFile,color.red(),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,color.green(),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,color.blue(),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,color.alpha(),addNewLine);
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const QColor &color)
{
    RFileIO::writeBinary(outFile,color.red());
    RFileIO::writeBinary(outFile,color.green());
    RFileIO::writeBinary(outFile,color.blue());
    RFileIO::writeBinary(outFile,color.alpha());
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RLight                                                           *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RGLLight &light)
{
    RFileIO::readAscii(inFile,light.enabled);
    RFileIO::readAscii(inFile,light.lightNum);
    RFileIO::readAscii(inFile,light.ambient);
    RFileIO::readAscii(inFile,light.diffuse);
    RFileIO::readAscii(inFile,light.specular);
    RFileIO::readAscii(inFile,light.direction);
    RFileIO::readAscii(inFile,light.position);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RGLLight &light)
{
    RFileIO::readBinary(inFile,light.enabled);
    RFileIO::readBinary(inFile,light.lightNum);
    RFileIO::readBinary(inFile,light.ambient);
    RFileIO::readBinary(inFile,light.diffuse);
    RFileIO::readBinary(inFile,light.specular);
    RFileIO::readBinary(inFile,light.direction);
    RFileIO::readBinary(inFile,light.position);
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RGLLight &light, bool addNewLine)
{
    RFileIO::writeAscii(outFile,light.enabled,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,light.lightNum,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,light.ambient,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,light.diffuse,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,light.specular,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,light.direction,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,light.position,addNewLine);
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RGLLight &light)
{
    RFileIO::writeBinary(outFile,light.enabled);
    RFileIO::writeBinary(outFile,light.lightNum);
    RFileIO::writeBinary(outFile,light.ambient);
    RFileIO::writeBinary(outFile,light.diffuse);
    RFileIO::writeBinary(outFile,light.specular);
    RFileIO::writeBinary(outFile,light.direction);
    RFileIO::writeBinary(outFile,light.position);
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RGLDisplayProperties                                             *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RGLDisplayProperties &displayProperties)
{
    unsigned int nLights;
    RFileIO::readAscii(inFile,displayProperties.drawGlobalAxis);
    RFileIO::readAscii(inFile,displayProperties.drawLocalAxis);
    RFileIO::readAscii(inFile,displayProperties.showModelEdges);
    RFileIO::readAscii(inFile,displayProperties.showModelDimensions);
    if (inFile.getVersion() > RVersion(1,0,0))
    {
        RFileIO::readAscii(inFile,displayProperties.showModelGrid);
    }
    RFileIO::readAscii(inFile,displayProperties.showErrors);
    RFileIO::readAscii(inFile,displayProperties.bgColor);
    RFileIO::readAscii(inFile,displayProperties.bgGradient);
    RFileIO::readAscii(inFile,nLights);
    displayProperties.setNLights(nLights);
    for (unsigned int i=0;i<nLights;i++)
    {
        RFileIO::readAscii(inFile,displayProperties.lights[i]);
    }
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RGLDisplayProperties &displayProperties)
{
    unsigned int nLights;
    RFileIO::readBinary(inFile,displayProperties.drawGlobalAxis);
    RFileIO::readBinary(inFile,displayProperties.drawLocalAxis);
    RFileIO::readBinary(inFile,displayProperties.showModelEdges);
    RFileIO::readBinary(inFile,displayProperties.showModelDimensions);
    if (inFile.getVersion() > RVersion(1,0,0))
    {
        RFileIO::readBinary(inFile,displayProperties.showModelGrid);
    }
    RFileIO::readBinary(inFile,displayProperties.showErrors);
    RFileIO::readBinary(inFile,displayProperties.bgColor);
    RFileIO::readBinary(inFile,displayProperties.bgGradient);
    RFileIO::readBinary(inFile,nLights);
    displayProperties.setNLights(nLights);
    for (unsigned int i=0;i<displayProperties.getNLights();i++)
    {
        RFileIO::readBinary(inFile,displayProperties.lights[i]);
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RGLDisplayProperties &displayProperties, bool addNewLine)
{
    RFileIO::writeAscii(outFile,displayProperties.drawGlobalAxis,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,displayProperties.drawLocalAxis,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,displayProperties.showModelEdges,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,displayProperties.showModelDimensions,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,displayProperties.showModelGrid,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,displayProperties.showErrors,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,displayProperties.bgColor,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,displayProperties.bgGradient,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,displayProperties.getNLights(),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    for (unsigned int i=0;i<displayProperties.getNLights();i++)
    {
        RFileIO::writeAscii(outFile,displayProperties.lights[i]);
        if (!addNewLine && i+1 < displayProperties.getNLights())
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RGLDisplayProperties &displayProperties)
{
    RFileIO::writeBinary(outFile,displayProperties.drawGlobalAxis);
    RFileIO::writeBinary(outFile,displayProperties.drawLocalAxis);
    RFileIO::writeBinary(outFile,displayProperties.showModelEdges);
    RFileIO::writeBinary(outFile,displayProperties.showModelDimensions);
    RFileIO::writeBinary(outFile,displayProperties.showModelGrid);
    RFileIO::writeBinary(outFile,displayProperties.showErrors);
    RFileIO::writeBinary(outFile,displayProperties.bgColor);
    RFileIO::writeBinary(outFile,displayProperties.bgGradient);
    RFileIO::writeBinary(outFile,displayProperties.getNLights());
    for (unsigned int i=0;i<displayProperties.getNLights();i++)
    {
        RFileIO::writeBinary(outFile,displayProperties.lights[i]);
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RModelData                                                       *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RModelData &modelData)
{
    RFileIO::readAscii(inFile,modelData.selected);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RModelData &modelData)
{
    RFileIO::readBinary(inFile,modelData.selected);
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RModelData &modelData, bool addNewLine)
{
    RFileIO::writeAscii(outFile,modelData.selected,addNewLine);
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RModelData &modelData)
{
    RFileIO::writeBinary(outFile,modelData.selected);
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RVariableType                                                    *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RVariableType &variableType)
{
    QString variableId;
    RFileIO::readAscii(inFile,variableId);
    variableType = RVariable::getTypeFromId(variableId);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RVariableType &variableType)
{
    QString variableId;
    RFileIO::readBinary(inFile,variableId);
    variableType = RVariable::getTypeFromId(variableId);
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RVariableType &variableType, bool addNewLine)
{
    RFileIO::writeAscii(outFile,RVariable::getId(variableType),addNewLine);
} /* RFileIO::writeAscii */

void RFileIO::writeAscii(RFile &outFile, const RVariableType &variableType, bool addNewLine)
{
    RFileIO::writeAscii(outFile,RVariable::getId(variableType),addNewLine);
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RVariableType &variableType)
{
    RFileIO::writeBinary(outFile,RVariable::getId(variableType));
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RVariableApplyType                                               *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RVariableApplyType &variableApplyType)
{
    int iValue;
    inFile.getTextStream() >> iValue;
    if (inFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF, "Failed to read RVariableApplyType value.");
    }
    variableApplyType = RVariableApplyType(iValue);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RVariableApplyType &variableApplyType)
{
    inFile.read((char*)&variableApplyType,sizeof(RVariableApplyType));
    if (inFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,"Failed to read RVariableApplyType value.");
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RVariableApplyType &variableApplyType, bool addNewLine)
{
    if (!addNewLine)
    {
        outFile.getTextStream() << int(variableApplyType);
    }
    else
    {
        outFile.getTextStream() << int(variableApplyType) << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write RVariableApplyType value.");
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RVariableApplyType &variableApplyType)
{
    outFile.write((char*)&variableApplyType,sizeof(RVariableApplyType));
    if (outFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write RVariableApplyType value.");
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RVariableData                                                    *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RVariableData &variableData)
{
    RFileIO::readAscii(inFile,variableData.minDisplayValue);
    RFileIO::readAscii(inFile,variableData.maxDisplayValue);
    RFileIO::readAscii(inFile,variableData.displayValueRange);
    RFileIO::readAscii(inFile,variableData.valueRangeName);
    RFileIO::readAscii(inFile,variableData.scale);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RVariableData &variableData)
{
    RFileIO::readBinary(inFile,variableData.minDisplayValue);
    RFileIO::readBinary(inFile,variableData.maxDisplayValue);
    RFileIO::readBinary(inFile,variableData.displayValueRange);
    RFileIO::readBinary(inFile,variableData.valueRangeName);
    RFileIO::readBinary(inFile,variableData.scale);
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RVariableData &variableData, bool addNewLine)
{
    RFileIO::writeAscii(outFile,variableData.minDisplayValue,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,variableData.maxDisplayValue,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,variableData.displayValueRange,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,variableData.valueRangeName,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,variableData.scale,addNewLine);
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RVariableData &variableData)
{
    RFileIO::writeBinary(outFile,variableData.minDisplayValue);
    RFileIO::writeBinary(outFile,variableData.maxDisplayValue);
    RFileIO::writeBinary(outFile,variableData.displayValueRange);
    RFileIO::writeBinary(outFile,variableData.valueRangeName);
    RFileIO::writeBinary(outFile,variableData.scale);
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RVariable                                                        *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RVariable &variable)
{
    RFileIO::readAscii(inFile,variable.type);
    RFileIO::readAscii(inFile,variable.applyType);
    RFileIO::readAscii(inFile,variable.name);
    RFileIO::readAscii(inFile,variable.units);
    unsigned int nValues = 0;
    RFileIO::readAscii(inFile,nValues);
    variable.values.resize(nValues);
    for (unsigned int i=0;i<variable.values.size();i++)
    {
        RFileIO::readAscii(inFile,variable.values[i]);
    }
    RFileIO::readAscii(inFile,variable.variableData);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RVariable &variable)
{
    RFileIO::readBinary(inFile,variable.type);
    RFileIO::readBinary(inFile,variable.applyType);
    RFileIO::readBinary(inFile,variable.name);
    RFileIO::readBinary(inFile,variable.units);
    unsigned int nValues = 0;
    RFileIO::readBinary(inFile,nValues);
    variable.values.resize(nValues);
    for (unsigned int i=0;i<variable.values.size();i++)
    {
        RFileIO::readBinary(inFile,variable.values[i]);
    }
    RFileIO::readBinary(inFile,variable.variableData);
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RVariable &variable, bool addNewLine)
{
    struct PrecisionGuard
    {
        QTextStream &stream;
        int previous;
        ~PrecisionGuard() { stream.setRealNumberPrecision(previous); }
    } precisionGuard{outFile.getTextStream(),outFile.getTextStream().realNumberPrecision()};
    // Result fields can be the input to a resumed solve. Keep double values
    // round-trip safe, as the model coordinates and restart records already are.
    outFile.getTextStream().setRealNumberPrecision(17);

    RFileIO::writeAscii(outFile,variable.type,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,variable.applyType,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    // The ASCII string reader treats whitespace as a delimiter unless quoted.
    // Result names such as "Von Mises stress" must remain a single field.
    RFileIO::writeAscii(outFile,"\"" + variable.name + "\"",addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,"\"" + variable.units + "\"",addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,(unsigned int)variable.values.size(),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    for (unsigned int i=0;i<variable.values.size();i++)
    {
        RFileIO::writeAscii(outFile,variable.values[i],addNewLine);
        if (!addNewLine && i+1 < variable.values.size())
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
    RFileIO::writeAscii(outFile,variable.variableData,addNewLine);
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RVariable &variable)
{
    RFileIO::writeBinary(outFile,variable.type);
    RFileIO::writeBinary(outFile,variable.applyType);
    RFileIO::writeBinary(outFile,variable.name);
    RFileIO::writeBinary(outFile,variable.units);
    RFileIO::writeBinary(outFile,(unsigned int)variable.values.size());
    for (unsigned int i=0;i<variable.values.size();i++)
    {
        RFileIO::writeBinary(outFile,variable.values[i]);
    }
    RFileIO::writeBinary(outFile,variable.variableData);
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RBoundaryConditionType                                           *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RBoundaryConditionType &bcType)
{
    QString propertyId;
    RFileIO::readAscii(inFile,propertyId);
    bcType = RBoundaryCondition::getTypeFromId(propertyId);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RBoundaryConditionType &bcType)
{
    QString propertyId;
    RFileIO::readBinary(inFile,propertyId);
    bcType = RBoundaryCondition::getTypeFromId(propertyId);
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RBoundaryConditionType &bcType, bool addNewLine)
{
    RFileIO::writeAscii(outFile,RBoundaryCondition::getId(bcType),addNewLine);
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RBoundaryConditionType &bcType)
{
    RFileIO::writeBinary(outFile,RBoundaryCondition::getId(bcType));
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RInitialConditionType                                            *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RInitialConditionType &icType)
{
    QString propertyId;
    RFileIO::readAscii(inFile,propertyId);
    icType = RInitialCondition::getTypeFromId(propertyId);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RInitialConditionType &icType)
{
    QString propertyId;
    RFileIO::readBinary(inFile,propertyId);
    icType = RInitialCondition::getTypeFromId(propertyId);
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RInitialConditionType &icType, bool addNewLine)
{
    RFileIO::writeAscii(outFile,RInitialCondition::getId(icType),addNewLine);
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RInitialConditionType &icType)
{
    RFileIO::writeBinary(outFile,RInitialCondition::getId(icType));
} /* RFileIO::writeBinary */


/*********************************************************************
 *  REnvironmentConditionType                                        *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, REnvironmentConditionType &ecType)
{
    QString propertyId;
    RFileIO::readAscii(inFile,propertyId);
    ecType = REnvironmentCondition::getTypeFromId(propertyId);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, REnvironmentConditionType &ecType)
{
    QString propertyId;
    RFileIO::readBinary(inFile,propertyId);
    ecType = REnvironmentCondition::getTypeFromId(propertyId);
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const REnvironmentConditionType &ecType, bool addNewLine)
{
    RFileIO::writeAscii(outFile,REnvironmentCondition::getId(ecType),addNewLine);
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const REnvironmentConditionType &ecType)
{
    RFileIO::writeBinary(outFile,REnvironmentCondition::getId(ecType));
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RConditionComponent                                              *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RConditionComponent &conditionComponent)
{
    RFileIO::readAscii(inFile,static_cast<RValueTable&>(conditionComponent));
    RFileIO::readAscii(inFile,conditionComponent.keyType);
    RFileIO::readAscii(inFile,conditionComponent.type);
    RFileIO::readAscii(inFile,conditionComponent.enabled);
}

void RFileIO::readBinary(RFile &inFile, RConditionComponent &conditionComponent)
{
    RFileIO::readBinary(inFile,static_cast<RValueTable&>(conditionComponent));
    RFileIO::readBinary(inFile,conditionComponent.keyType);
    RFileIO::readBinary(inFile,conditionComponent.type);
    RFileIO::readBinary(inFile,conditionComponent.enabled);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RConditionComponent &conditionComponent, bool addNewLine)
{
    RFileIO::writeAscii(outFile,static_cast<const RValueTable&>(conditionComponent),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,conditionComponent.keyType,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,conditionComponent.type,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,conditionComponent.enabled,addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RConditionComponent &conditionComponent)
{
    RFileIO::writeBinary(outFile,static_cast<const RValueTable&>(conditionComponent));
    RFileIO::writeBinary(outFile,conditionComponent.keyType);
    RFileIO::writeBinary(outFile,conditionComponent.type);
    RFileIO::writeBinary(outFile,conditionComponent.enabled);
}


/*********************************************************************
 *  RCondition                                                       *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RCondition &condition)
{
    RFileIO::readAscii(inFile,condition.name);
    unsigned int nComponents;
    RFileIO::readAscii(inFile,nComponents);
    condition.components.resize(nComponents);
    for (unsigned int i=0;i<nComponents;i++)
    {
        RFileIO::readAscii(inFile,condition.components[i]);
    }
    RFileIO::readAscii(inFile,condition.applyOnPoint);
    RFileIO::readAscii(inFile,condition.applyOnLine);
    RFileIO::readAscii(inFile,condition.applyOnSurface);
    RFileIO::readAscii(inFile,condition.applyOnVolume);
    RFileIO::readAscii(inFile,condition.problemTypeMask);
}


void RFileIO::readBinary(RFile &inFile, RCondition &condition)
{
    RFileIO::readBinary(inFile,condition.name);
    unsigned int nComponents;
    RFileIO::readBinary(inFile,nComponents);
    condition.components.resize(nComponents);
    for (unsigned int i=0;i<nComponents;i++)
    {
        RFileIO::readBinary(inFile,condition.components[i]);
    }
    RFileIO::readBinary(inFile,condition.applyOnPoint);
    RFileIO::readBinary(inFile,condition.applyOnLine);
    RFileIO::readBinary(inFile,condition.applyOnSurface);
    RFileIO::readBinary(inFile,condition.applyOnVolume);
    RFileIO::readBinary(inFile,condition.problemTypeMask);
}


void RFileIO::writeAscii(RSaveFile &outFile, const RCondition &condition, bool addNewLine)
{
    RFileIO::writeAscii(outFile,"\"" + condition.name + "\"",addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,uint(condition.size()),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    for (unsigned int i=0;i<condition.components.size();i++)
    {
        RFileIO::writeAscii(outFile,condition.components[i],addNewLine);
        if (!addNewLine)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
    RFileIO::writeAscii(outFile,condition.applyOnPoint,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,condition.applyOnLine,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,condition.applyOnSurface,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,condition.applyOnVolume,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,condition.problemTypeMask,addNewLine);
}


void RFileIO::writeBinary(RSaveFile &outFile, const RCondition &condition)
{
    RFileIO::writeBinary(outFile,condition.name);
    RFileIO::writeBinary(outFile,uint(condition.size()));
    for (unsigned int i=0;i<condition.components.size();i++)
    {
        RFileIO::writeBinary(outFile,condition.components[i]);
    }
    RFileIO::writeBinary(outFile,condition.applyOnPoint);
    RFileIO::writeBinary(outFile,condition.applyOnLine);
    RFileIO::writeBinary(outFile,condition.applyOnSurface);
    RFileIO::writeBinary(outFile,condition.applyOnVolume);
    RFileIO::writeBinary(outFile,condition.problemTypeMask);
}


/*********************************************************************
 *  RBoundaryCondition                                               *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RBoundaryCondition &boundaryCondition)
{
    RFileIO::readAscii(inFile,static_cast<RCondition&>(boundaryCondition));
    RFileIO::readAscii(inFile,boundaryCondition.type);
    RFileIO::readAscii(inFile,boundaryCondition.optional);
    RFileIO::readAscii(inFile,boundaryCondition.isExplicit);
    RFileIO::readAscii(inFile,boundaryCondition.hasLocalDirection);
    RFileIO::readAscii(inFile,boundaryCondition.direction[0]);
    RFileIO::readAscii(inFile,boundaryCondition.direction[1]);
    RFileIO::readAscii(inFile,boundaryCondition.direction[2]);
    if (inFile.getVersion() > RVersion(1,2,0))
    {
        RFileIO::readAscii(inFile,boundaryCondition.explicitLocalDirection);
    }
}


void RFileIO::readBinary(RFile &inFile, RBoundaryCondition &boundaryCondition)
{
    RFileIO::readBinary(inFile,static_cast<RCondition&>(boundaryCondition));
    RFileIO::readBinary(inFile,boundaryCondition.type);
    RFileIO::readBinary(inFile,boundaryCondition.optional);
    RFileIO::readBinary(inFile,boundaryCondition.isExplicit);
    RFileIO::readBinary(inFile,boundaryCondition.hasLocalDirection);
    RFileIO::readBinary(inFile,boundaryCondition.direction[0]);
    RFileIO::readBinary(inFile,boundaryCondition.direction[1]);
    RFileIO::readBinary(inFile,boundaryCondition.direction[2]);
    if (inFile.getVersion() > RVersion(1,2,0))
    {
        RFileIO::readBinary(inFile,boundaryCondition.explicitLocalDirection);
    }
}


void RFileIO::writeAscii(RSaveFile &outFile, const RBoundaryCondition &boundaryCondition, bool addNewLine)
{
    RFileIO::writeAscii(outFile,static_cast<const RCondition&>(boundaryCondition),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,boundaryCondition.type,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,boundaryCondition.optional,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,boundaryCondition.isExplicit,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,boundaryCondition.hasLocalDirection,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,boundaryCondition.direction[0],addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,boundaryCondition.direction[1],addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,boundaryCondition.direction[2],addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,boundaryCondition.explicitLocalDirection,addNewLine);
}


void RFileIO::writeBinary(RSaveFile &outFile, const RBoundaryCondition &boundaryCondition)
{
    RFileIO::writeBinary(outFile,static_cast<const RCondition&>(boundaryCondition));
    RFileIO::writeBinary(outFile,boundaryCondition.type);
    RFileIO::writeBinary(outFile,boundaryCondition.optional);
    RFileIO::writeBinary(outFile,boundaryCondition.isExplicit);
    RFileIO::writeBinary(outFile,boundaryCondition.hasLocalDirection);
    RFileIO::writeBinary(outFile,boundaryCondition.direction[0]);
    RFileIO::writeBinary(outFile,boundaryCondition.direction[1]);
    RFileIO::writeBinary(outFile,boundaryCondition.direction[2]);
    RFileIO::writeBinary(outFile,boundaryCondition.explicitLocalDirection);
}


/*********************************************************************
 *  RInitialCondition                                                *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RInitialCondition &initialCondition)
{
    RFileIO::readAscii(inFile,static_cast<RCondition&>(initialCondition));
    RFileIO::readAscii(inFile,initialCondition.type);
}


void RFileIO::readBinary(RFile &inFile, RInitialCondition &initialCondition)
{
    RFileIO::readBinary(inFile,static_cast<RCondition&>(initialCondition));
    RFileIO::readBinary(inFile,initialCondition.type);
}


void RFileIO::writeAscii(RSaveFile &outFile, const RInitialCondition &initialCondition, bool addNewLine)
{
    RFileIO::writeAscii(outFile,static_cast<const RCondition&>(initialCondition),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,initialCondition.type,addNewLine);
}


void RFileIO::writeBinary(RSaveFile &outFile, const RInitialCondition &initialCondition)
{
    RFileIO::writeBinary(outFile,static_cast<const RCondition&>(initialCondition));
    RFileIO::writeBinary(outFile,initialCondition.type);
}


/*********************************************************************
 *  REnvironmentCondition                                            *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, REnvironmentCondition &environmentCondition)
{
    RFileIO::readAscii(inFile,static_cast<RCondition&>(environmentCondition));
    RFileIO::readAscii(inFile,environmentCondition.type);
}


void RFileIO::readBinary(RFile &inFile, REnvironmentCondition &environmentCondition)
{
    RFileIO::readBinary(inFile,static_cast<RCondition&>(environmentCondition));
    RFileIO::readBinary(inFile,environmentCondition.type);
}


void RFileIO::writeAscii(RSaveFile &outFile, const REnvironmentCondition &environmentCondition, bool addNewLine)
{
    RFileIO::writeAscii(outFile,static_cast<const RCondition&>(environmentCondition),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,environmentCondition.type,addNewLine);
}


void RFileIO::writeBinary(RSaveFile &outFile, const REnvironmentCondition &environmentCondition)
{
    RFileIO::writeBinary(outFile,static_cast<const RCondition&>(environmentCondition));
    RFileIO::writeBinary(outFile,environmentCondition.type);
}


/*********************************************************************
 *  RMaterialProperty::Type                                            *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RMaterialProperty::Type &mpType)
{
    QString propertyId;
    RFileIO::readAscii(inFile,propertyId);
    mpType = RMaterialProperty::getTypeFromId(propertyId);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RMaterialProperty::Type &mpType)
{
    QString propertyId;
    RFileIO::readBinary(inFile,propertyId);
    mpType = RMaterialProperty::getTypeFromId(propertyId);
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RMaterialProperty::Type &mpType, bool addNewLine)
{
    RFileIO::writeAscii(outFile,RMaterialProperty::getId(mpType),addNewLine);
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RMaterialProperty::Type &mpType)
{
    RFileIO::writeBinary(outFile,RMaterialProperty::getId(mpType));

} /* RFileIO::writeBinary */


/*********************************************************************
 *  RMaterial::State                                                   *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RMaterial::State &state)
{
    int iValue;
    inFile.getTextStream() >> iValue;
    if (inFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF, "Failed to read RMaterial::State value.");
    }
    state = RMaterial::State(iValue);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RMaterial::State &state)
{
    inFile.read((char*)&state,sizeof(RMaterial::State));
    if (inFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,"Failed to read RMaterial::State value.");
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RMaterial::State &state, bool addNewLine)
{
    if (!addNewLine)
    {
        outFile.getTextStream() << int(state);
    }
    else
    {
        outFile.getTextStream() << int(state) << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write RMaterial::State value.");
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RMaterial::State &state)
{
    outFile.write((char*)&state,sizeof(RMaterial::State));
    if (outFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write RMaterial::State value.");
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RMaterialProperty                                                *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RMaterialProperty &materialProperty)
{
    RFileIO::readAscii(inFile,static_cast<RValueTable&>(materialProperty));
    RFileIO::readAscii(inFile,materialProperty.keyType);
    RFileIO::readAscii(inFile,materialProperty.type);
    // HACK - always based on temperature
    materialProperty.keyType = R_VARIABLE_TEMPERATURE;
}

void RFileIO::readBinary(RFile &inFile, RMaterialProperty &materialProperty)
{
    RFileIO::readBinary(inFile,static_cast<RValueTable&>(materialProperty));
    RFileIO::readBinary(inFile,materialProperty.keyType);
    RFileIO::readBinary(inFile,materialProperty.type);
    // HACK - always based on temperature
    materialProperty.keyType = R_VARIABLE_TEMPERATURE;
}

void RFileIO::writeAscii(RSaveFile &outFile, const RMaterialProperty &materialProperty, bool addNewLine)
{
    RFileIO::writeAscii(outFile,static_cast<const RValueTable&>(materialProperty),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,materialProperty.keyType,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,materialProperty.type,addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RMaterialProperty &materialProperty)
{
    RFileIO::writeBinary(outFile,static_cast<const RValueTable&>(materialProperty));
    RFileIO::writeBinary(outFile,materialProperty.keyType);
    RFileIO::writeBinary(outFile,materialProperty.type);
}


/*********************************************************************
 *  RMaterial                                                        *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RMaterial &material)
{
    RFileIO::readAscii(inFile,material.id);
    RFileIO::readAscii(inFile,material.name);
    RFileIO::readAscii(inFile,material.state);
    unsigned int nPorperties;
    RFileIO::readAscii(inFile,nPorperties);
    material.properties.resize(nPorperties);
    for (unsigned int i=0;i<nPorperties;i++)
    {
        RFileIO::readAscii(inFile,material.properties[i]);
    }
}


void RFileIO::readBinary(RFile &inFile, RMaterial &material)
{
    if (inFile.getVersion() > RVersion("1.1.0"))
    {
        RFileIO::readBinary(inFile,material.id);
    }
    else
    {
        int id;
        RFileIO::readBinary(inFile,id);
        material.id = QUuid::createUuid();
    }
    RFileIO::readBinary(inFile,material.name);
    RFileIO::readBinary(inFile,material.state);
    unsigned int nPorperties;
    RFileIO::readBinary(inFile,nPorperties);
    material.properties.resize(nPorperties);
    for (unsigned int i=0;i<nPorperties;i++)
    {
        RFileIO::readBinary(inFile,material.properties[i]);
    }
}


void RFileIO::writeAscii(RSaveFile &outFile, const RMaterial &material, bool addNewLine)
{
    RFileIO::writeAscii(outFile,material.id,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,"\"" + material.name + "\"",addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,material.state,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,uint(material.size()),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    for (unsigned int i=0;i<material.properties.size();i++)
    {
        RFileIO::writeAscii(outFile,material.properties[i],addNewLine);
        if (!addNewLine && i+1 < material.properties.size())
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
}


void RFileIO::writeBinary(RSaveFile &outFile, const RMaterial &material)
{
    RFileIO::writeBinary(outFile,material.id);
    RFileIO::writeBinary(outFile,material.name);
    RFileIO::writeBinary(outFile,material.state);
    RFileIO::writeBinary(outFile,uint(material.size()));
    for (unsigned int i=0;i<material.properties.size();i++)
    {
        RFileIO::writeBinary(outFile,material.properties[i]);
    }
}


/*********************************************************************
 *  RNode                                                            *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RNode &node)
{
    RFileIO::readAscii(inFile,node.x);
    RFileIO::readAscii(inFile,node.y);
    RFileIO::readAscii(inFile,node.z);
}


void RFileIO::readBinary(RFile &inFile, RNode &node)
{
    RFileIO::readBinary(inFile,node.x);
    RFileIO::readBinary(inFile,node.y);
    RFileIO::readBinary(inFile,node.z);
}


void RFileIO::writeAscii(RSaveFile &outFile, const RNode &node, bool addNewLine)
{
    struct PrecisionGuard
    {
        QTextStream &stream;
        int previous;
        ~PrecisionGuard() { stream.setRealNumberPrecision(previous); }
    } precisionGuard{outFile.getTextStream(),outFile.getTextStream().realNumberPrecision()};
    // Preserve exact model coordinates so exact mesh bindings survive ASCII round trips.
    outFile.getTextStream().setRealNumberPrecision(17);

    RFileIO::writeAscii(outFile,node.x,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,node.y,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,node.z,addNewLine);
}


void RFileIO::writeBinary(RSaveFile &outFile, const RNode &node)
{
    RFileIO::writeBinary(outFile,node.x);
    RFileIO::writeBinary(outFile,node.y);
    RFileIO::writeBinary(outFile,node.z);
}


/*********************************************************************
 *  RElementType                                                     *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RElementType &elementType)
{
    QString propertyId;
    RFileIO::readAscii(inFile,propertyId);
    elementType = RElement::getTypeFromId(propertyId);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RElementType &elementType)
{
    QString propertyId;
    RFileIO::readBinary(inFile,propertyId);
    elementType = RElement::getTypeFromId(propertyId);
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RElementType &elementType, bool addNewLine)
{
    RFileIO::writeAscii(outFile,RElement::getId(elementType),addNewLine);
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RElementType &elementType)
{
    RFileIO::writeBinary(outFile,RElement::getId(elementType));
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RElement                                                         *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RElement &element)
{
    RFileIO::readAscii(inFile,element.type);
    unsigned int nNodeIDs;
    RFileIO::readAscii(inFile,nNodeIDs);
    element.nodeIDs.resize(nNodeIDs);
    for (unsigned int i=0;i<nNodeIDs;i++)
    {
        RFileIO::readAscii(inFile,element.nodeIDs[i]);
    }
}


void RFileIO::readBinary(RFile &inFile, RElement &element)
{
    RFileIO::readBinary(inFile,element.type);
    unsigned int nNodeIDs;
    RFileIO::readBinary(inFile,nNodeIDs);
    element.nodeIDs.resize(nNodeIDs);
    for (unsigned int i=0;i<nNodeIDs;i++)
    {
        RFileIO::readBinary(inFile,element.nodeIDs[i]);
    }
}


void RFileIO::writeAscii(RSaveFile &outFile, const RElement &element, bool addNewLine)
{
    RFileIO::writeAscii(outFile,element.type,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,uint(element.size()),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    for (unsigned int i=0;i<element.nodeIDs.size();i++)
    {
        RFileIO::writeAscii(outFile,element.nodeIDs[i],addNewLine);
        if (!addNewLine && i+1 < element.nodeIDs.size())
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
}


void RFileIO::writeBinary(RSaveFile &outFile, const RElement &element)
{
    RFileIO::writeBinary(outFile,element.type);
    RFileIO::writeBinary(outFile,uint(element.size()));
    for (unsigned int i=0;i<element.nodeIDs.size();i++)
    {
        RFileIO::writeBinary(outFile,element.nodeIDs[i]);
    }
}


/*********************************************************************
 *  RElementGroupVariableDisplayType                                 *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, REntityGroupVariableDisplayType &elementGroupVariableDisplayType)
{
    int iValue;
    inFile.getTextStream() >> iValue;
    if (inFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF, "Failed to read RElementGroupVariableDisplayType value.");
    }
    elementGroupVariableDisplayType = REntityGroupVariableDisplayType(iValue);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, REntityGroupVariableDisplayType &elementGroupVariableDisplayType)
{
    inFile.read((char*)&elementGroupVariableDisplayType,sizeof(REntityGroupVariableDisplayType));
    if (inFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,"Failed to read RElementGroupVariableDisplayType value.");
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const REntityGroupVariableDisplayType &elementGroupVariableDisplayType, bool addNewLine)
{
    if (!addNewLine)
    {
        outFile.getTextStream() << int(elementGroupVariableDisplayType);
    }
    else
    {
        outFile.getTextStream() << int(elementGroupVariableDisplayType) << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write RElementGroupVariableDisplayType value.");
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const REntityGroupVariableDisplayType &elementGroupVariableDisplayType)
{
    outFile.write((char*)&elementGroupVariableDisplayType,sizeof(REntityGroupVariableDisplayType));
    if (outFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write RElementGroupVariableDisplayType value.");
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RElementGroupData                                                *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, REntityGroupData &elementGroupData)
{
    RFileIO::readAscii(inFile,elementGroupData.selected);
    RFileIO::readAscii(inFile,elementGroupData.visible);
    RFileIO::readAscii(inFile,elementGroupData.rgba[0]);
    RFileIO::readAscii(inFile,elementGroupData.rgba[1]);
    RFileIO::readAscii(inFile,elementGroupData.rgba[2]);
    RFileIO::readAscii(inFile,elementGroupData.rgba[3]);
    RFileIO::readAscii(inFile,elementGroupData.drawWire);
    RFileIO::readAscii(inFile,elementGroupData.drawEdges);
    RFileIO::readAscii(inFile,elementGroupData.drawNodes);
    RFileIO::readAscii(inFile,elementGroupData.drawElementNumbers);
    RFileIO::readAscii(inFile,elementGroupData.drawNodeNumbers);
    RFileIO::readAscii(inFile,elementGroupData.drawArrowHeads);
    RFileIO::readAscii(inFile,elementGroupData.drawEqualArrowLength);
    RFileIO::readAscii(inFile,elementGroupData.drawArrowFrom);
    RFileIO::readAscii(inFile,elementGroupData.colorByPatch);
    RFileIO::readAscii(inFile,elementGroupData.colorByViewFactor);
    elementGroupData.variableData.clear();
    unsigned int nVariableData = 0;
    RFileIO::readAscii(inFile,nVariableData);
    for (unsigned int i=0;i<nVariableData;i++)
    {
        RVariableType variableType;
        REntityGroupVariableDisplayType variableDisplayType;
        RFileIO::readAscii(inFile,variableType);
        RFileIO::readAscii(inFile,variableDisplayType);
        elementGroupData.variableData[variableType] = variableDisplayType;
    }
}


void RFileIO::readBinary(RFile &inFile, REntityGroupData &elementGroupData)
{
    RFileIO::readBinary(inFile,elementGroupData.selected);
    RFileIO::readBinary(inFile,elementGroupData.visible);
    RFileIO::readBinary(inFile,elementGroupData.rgba[0]);
    RFileIO::readBinary(inFile,elementGroupData.rgba[1]);
    RFileIO::readBinary(inFile,elementGroupData.rgba[2]);
    RFileIO::readBinary(inFile,elementGroupData.rgba[3]);
    RFileIO::readBinary(inFile,elementGroupData.drawWire);
    RFileIO::readBinary(inFile,elementGroupData.drawEdges);
    RFileIO::readBinary(inFile,elementGroupData.drawNodes);
    RFileIO::readBinary(inFile,elementGroupData.drawElementNumbers);
    RFileIO::readBinary(inFile,elementGroupData.drawNodeNumbers);
    RFileIO::readBinary(inFile,elementGroupData.drawArrowHeads);
    RFileIO::readBinary(inFile,elementGroupData.drawEqualArrowLength);
    RFileIO::readBinary(inFile,elementGroupData.drawArrowFrom);
    RFileIO::readBinary(inFile,elementGroupData.colorByPatch);
    RFileIO::readBinary(inFile,elementGroupData.colorByViewFactor);
    elementGroupData.variableData.clear();
    unsigned int nVariableData = 0;
    RFileIO::readBinary(inFile,nVariableData);
    for (unsigned int i=0;i<nVariableData;i++)
    {
        RVariableType variableType;
        REntityGroupVariableDisplayType variableDisplayType;
        RFileIO::readBinary(inFile,variableType);
        RFileIO::readBinary(inFile,variableDisplayType);
        elementGroupData.variableData[variableType] = variableDisplayType;
    }
}


void RFileIO::writeAscii(RSaveFile &outFile, const REntityGroupData &elementGroupData, bool addNewLine)
{
    RFileIO::writeAscii(outFile,elementGroupData.selected,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,elementGroupData.visible,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,elementGroupData.rgba[0],addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,elementGroupData.rgba[1],addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,elementGroupData.rgba[2],addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,elementGroupData.rgba[3],addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,elementGroupData.drawWire,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,elementGroupData.drawEdges,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,elementGroupData.drawNodes,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,elementGroupData.drawElementNumbers,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,elementGroupData.drawNodeNumbers,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,elementGroupData.drawArrowHeads,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,elementGroupData.drawEqualArrowLength,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,elementGroupData.drawArrowFrom,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,elementGroupData.colorByPatch,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,elementGroupData.colorByViewFactor,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,(unsigned int)elementGroupData.variableData.size(),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    typedef std::map<RVariableType,REntityGroupVariableDisplayTypeMask>::const_iterator it_type;
    for (it_type iterator = elementGroupData.variableData.begin(); iterator != elementGroupData.variableData.end(); iterator++) {
        RFileIO::writeAscii(outFile,RVariableType(iterator->first),addNewLine);
        if (!addNewLine)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
        RFileIO::writeAscii(outFile,REntityGroupVariableDisplayTypeMask(iterator->second),addNewLine);
        if (!addNewLine)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
}


void RFileIO::writeBinary(RSaveFile &outFile, const REntityGroupData &elementGroupData)
{
    RFileIO::writeBinary(outFile,elementGroupData.selected);
    RFileIO::writeBinary(outFile,elementGroupData.visible);
    RFileIO::writeBinary(outFile,elementGroupData.rgba[0]);
    RFileIO::writeBinary(outFile,elementGroupData.rgba[1]);
    RFileIO::writeBinary(outFile,elementGroupData.rgba[2]);
    RFileIO::writeBinary(outFile,elementGroupData.rgba[3]);
    RFileIO::writeBinary(outFile,elementGroupData.drawWire);
    RFileIO::writeBinary(outFile,elementGroupData.drawEdges);
    RFileIO::writeBinary(outFile,elementGroupData.drawNodes);
    RFileIO::writeBinary(outFile,elementGroupData.drawElementNumbers);
    RFileIO::writeBinary(outFile,elementGroupData.drawNodeNumbers);
    RFileIO::writeBinary(outFile,elementGroupData.drawArrowHeads);
    RFileIO::writeBinary(outFile,elementGroupData.drawEqualArrowLength);
    RFileIO::writeBinary(outFile,elementGroupData.drawArrowFrom);
    RFileIO::writeBinary(outFile,elementGroupData.colorByPatch);
    RFileIO::writeBinary(outFile,elementGroupData.colorByViewFactor);
    RFileIO::writeBinary(outFile,(unsigned int)elementGroupData.variableData.size());
    typedef std::map<RVariableType,REntityGroupVariableDisplayTypeMask>::const_iterator it_type;
    for (it_type iterator = elementGroupData.variableData.begin(); iterator != elementGroupData.variableData.end(); iterator++) {
        RFileIO::writeBinary(outFile,RVariableType(iterator->first));
        RFileIO::writeBinary(outFile,REntityGroupVariableDisplayTypeMask(iterator->second));
    }
}


/*********************************************************************
 *  REntityGroup                                                     *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, REntityGroup &entityGroup)
{
    RFileIO::readAscii(inFile,entityGroup.name);
    RFileIO::readAscii(inFile,entityGroup.data);
}

void RFileIO::readBinary(RFile &inFile, REntityGroup &entityGroup)
{
    RFileIO::readBinary(inFile,entityGroup.name);
    RFileIO::readBinary(inFile,entityGroup.data);
}

void RFileIO::writeAscii(RSaveFile &outFile, const REntityGroup &entityGroup, bool addNewLine)
{
    RFileIO::writeAscii(outFile,"\"" + entityGroup.name + "\"",addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,entityGroup.data,addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const REntityGroup &entityGroup)
{
    RFileIO::writeBinary(outFile,entityGroup.name);
    RFileIO::writeBinary(outFile,entityGroup.data);
}


/*********************************************************************
 *  RElementGroup                                                    *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RElementGroup &elementGroup)
{
    if (inFile.getVersion() >= RVersion(1,3,18))
    {
        RFileIO::readAscii(inFile,elementGroup.physicalId);
    }
    else
    {
        elementGroup.physicalId = 0;
    }
    RFileIO::readAscii(inFile,static_cast<REntityGroup&>(elementGroup));
    unsigned int nElementIDs = 0;
    RFileIO::readAscii(inFile,nElementIDs);
    elementGroup.elementIDs.resize(nElementIDs);
    for (unsigned int i=0;i<nElementIDs;i++)
    {
        RFileIO::readAscii(inFile,elementGroup.elementIDs[i]);
    }
    // The serialized condition counts describe the complete group state.
    // Clear constructor defaults and prior contents before applying that snapshot.
    elementGroup.bConditions.clear();
    elementGroup.eConditions.clear();
    elementGroup.iConditions.clear();
    unsigned int nBConditions = 0;
    RFileIO::readAscii(inFile,nBConditions);
    RBoundaryCondition bCondition;
    for (unsigned int i=0;i<nBConditions;i++)
    {
        RFileIO::readAscii(inFile,bCondition);
        elementGroup.addBoundaryCondition(bCondition);
    }
    unsigned int nEConditions = 0;
    RFileIO::readAscii(inFile,nEConditions);
    REnvironmentCondition eCondition;
    for (unsigned int i=0;i<nEConditions;i++)
    {
        RFileIO::readAscii(inFile,eCondition);
        elementGroup.addEnvironmentCondition(eCondition);
    }
    unsigned int nIConditions = 0;
    RFileIO::readAscii(inFile,nIConditions);
    RInitialCondition iCondition;
    for (unsigned int i=0;i<nIConditions;i++)
    {
        RFileIO::readAscii(inFile,iCondition);
        elementGroup.addInitialCondition(iCondition);
    }
    RFileIO::readAscii(inFile,elementGroup.material);
}


void RFileIO::readBinary(RFile &inFile, RElementGroup &elementGroup)
{
    if (inFile.getVersion() >= RVersion(1,3,18))
    {
        RFileIO::readBinary(inFile,elementGroup.physicalId);
    }
    else
    {
        elementGroup.physicalId = 0;
    }
    RFileIO::readBinary(inFile,static_cast<REntityGroup&>(elementGroup));
    unsigned int nElementIDs = 0;
    RFileIO::readBinary(inFile,nElementIDs);
    elementGroup.elementIDs.resize(nElementIDs);
    for (unsigned int i=0;i<nElementIDs;i++)
    {
        RFileIO::readBinary(inFile,elementGroup.elementIDs[i]);
    }
    // The serialized condition counts describe the complete group state.
    // Clear constructor defaults and prior contents before applying that snapshot.
    elementGroup.bConditions.clear();
    elementGroup.eConditions.clear();
    elementGroup.iConditions.clear();
    unsigned int nBConditions = 0;
    RFileIO::readBinary(inFile,nBConditions);
    RBoundaryCondition bCondition;
    for (unsigned int i=0;i<nBConditions;i++)
    {
        RFileIO::readBinary(inFile,bCondition);
        elementGroup.addBoundaryCondition(bCondition);
    }
    unsigned int nEConditions = 0;
    RFileIO::readBinary(inFile,nEConditions);
    REnvironmentCondition eCondition;
    for (unsigned int i=0;i<nEConditions;i++)
    {
        RFileIO::readBinary(inFile,eCondition);
        elementGroup.addEnvironmentCondition(eCondition);
    }
    unsigned int nIConditions = 0;
    RFileIO::readBinary(inFile,nIConditions);
    RInitialCondition iCondition;
    for (unsigned int i=0;i<nIConditions;i++)
    {
        RFileIO::readBinary(inFile,iCondition);
        elementGroup.addInitialCondition(iCondition);
    }
    RFileIO::readBinary(inFile,elementGroup.material);
}


void RFileIO::writeAscii(RSaveFile &outFile, const RElementGroup &elementGroup, bool addNewLine)
{
    if (outFile.getVersion() >= RVersion(1,3,18))
    {
        RFileIO::writeAscii(outFile,elementGroup.physicalId,addNewLine);
        if (!addNewLine)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
    RFileIO::writeAscii(outFile,static_cast<const REntityGroup&>(elementGroup),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,uint(elementGroup.size()),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    for (unsigned int i=0;i<elementGroup.elementIDs.size();i++)
    {
        RFileIO::writeAscii(outFile,elementGroup.elementIDs[i],addNewLine);
        if (!addNewLine)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
    RFileIO::writeAscii(outFile,elementGroup.getNBoundaryConditions(),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    for (unsigned int i=0;i<elementGroup.bConditions.size();i++)
    {
        RFileIO::writeAscii(outFile,elementGroup.getBoundaryCondition(i),addNewLine);
        if (!addNewLine)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
    RFileIO::writeAscii(outFile,elementGroup.getNEnvironmentConditions(),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    for (unsigned int i=0;i<elementGroup.eConditions.size();i++)
    {
        RFileIO::writeAscii(outFile,elementGroup.getEnvironmentCondition(i),addNewLine);
        if (!addNewLine)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
    RFileIO::writeAscii(outFile,elementGroup.getNInitialConditions(),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    for (unsigned int i=0;i<elementGroup.iConditions.size();i++)
    {
        RFileIO::writeAscii(outFile,elementGroup.getInitialCondition(i),addNewLine);
        if (!addNewLine)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
    RFileIO::writeAscii(outFile,elementGroup.material,addNewLine);
}


void RFileIO::writeBinary(RSaveFile &outFile, const RElementGroup &elementGroup)
{
    if (outFile.getVersion() >= RVersion(1,3,18))
    {
        RFileIO::writeBinary(outFile,elementGroup.physicalId);
    }
    RFileIO::writeBinary(outFile,static_cast<const REntityGroup&>(elementGroup));
    RFileIO::writeBinary(outFile,uint(elementGroup.size()));
    for (unsigned int i=0;i<elementGroup.elementIDs.size();i++)
    {
        RFileIO::writeBinary(outFile,elementGroup.elementIDs[i]);
    }
    RFileIO::writeBinary(outFile,elementGroup.getNBoundaryConditions());
    for (unsigned int i=0;i<elementGroup.bConditions.size();i++)
    {
        RFileIO::writeBinary(outFile,elementGroup.getBoundaryCondition(i));
    }
    RFileIO::writeBinary(outFile,elementGroup.getNEnvironmentConditions());
    for (unsigned int i=0;i<elementGroup.eConditions.size();i++)
    {
        RFileIO::writeBinary(outFile,elementGroup.getEnvironmentCondition(i));
    }
    RFileIO::writeBinary(outFile,elementGroup.getNInitialConditions());
    for (unsigned int i=0;i<elementGroup.iConditions.size();i++)
    {
        RFileIO::writeBinary(outFile,elementGroup.getInitialCondition(i));
    }
    RFileIO::writeBinary(outFile,elementGroup.material);
}


/*********************************************************************
 *  RPoint                                                           *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RPoint &point)
{
    RFileIO::readAscii(inFile,static_cast<RElementGroup&>(point));
    RFileIO::readAscii(inFile,point.volume);
}

void RFileIO::readBinary(RFile &inFile, RPoint &point)
{
    RFileIO::readBinary(inFile,static_cast<RElementGroup&>(point));
    RFileIO::readBinary(inFile,point.volume);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RPoint &point, bool addNewLine)
{
    RFileIO::writeAscii(outFile,static_cast<const RElementGroup&>(point),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,point.volume,addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RPoint &point)
{
    RFileIO::writeBinary(outFile,static_cast<const RElementGroup&>(point));
    RFileIO::writeBinary(outFile,point.volume);
}


/*********************************************************************
 *  RLine                                                            *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RLine &line)
{
    RFileIO::readAscii(inFile,static_cast<RElementGroup&>(line));
    RFileIO::readAscii(inFile,line.crossArea);
    if (inFile.getVersion() > RVersion(1,3,2))
    {
        RFileIO::readAscii(inFile,line.secondMomentY);
        RFileIO::readAscii(inFile,line.secondMomentZ);
        RFileIO::readAscii(inFile,line.torsionalConstant);
        RFileIO::readAscii(inFile,line.localYDirection);
    }
    if (inFile.getVersion() > RVersion(1,3,3))
    {
        RFileIO::readAscii(inFile,line.sectionModulusY);
        RFileIO::readAscii(inFile,line.sectionModulusZ);
        RFileIO::readAscii(inFile,line.torsionalSectionModulus);
    }
}

void RFileIO::readBinary(RFile &inFile, RLine &line)
{
    RFileIO::readBinary(inFile,static_cast<RElementGroup&>(line));
    RFileIO::readBinary(inFile,line.crossArea);
    if (inFile.getVersion() > RVersion(1,3,2))
    {
        RFileIO::readBinary(inFile,line.secondMomentY);
        RFileIO::readBinary(inFile,line.secondMomentZ);
        RFileIO::readBinary(inFile,line.torsionalConstant);
        RFileIO::readBinary(inFile,line.localYDirection);
    }
    if (inFile.getVersion() > RVersion(1,3,3))
    {
        RFileIO::readBinary(inFile,line.sectionModulusY);
        RFileIO::readBinary(inFile,line.sectionModulusZ);
        RFileIO::readBinary(inFile,line.torsionalSectionModulus);
    }
}

void RFileIO::writeAscii(RSaveFile &outFile, const RLine &line, bool addNewLine)
{
    RFileIO::writeAscii(outFile,static_cast<const RElementGroup&>(line),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,line.crossArea,false);
    RFileIO::writeAscii(outFile,' ',false);
    RFileIO::writeAscii(outFile,line.secondMomentY,false);
    RFileIO::writeAscii(outFile,' ',false);
    RFileIO::writeAscii(outFile,line.secondMomentZ,false);
    RFileIO::writeAscii(outFile,' ',false);
    RFileIO::writeAscii(outFile,line.torsionalConstant,false);
    RFileIO::writeAscii(outFile,' ',false);
    RFileIO::writeAscii(outFile,line.localYDirection,false);
    RFileIO::writeAscii(outFile,' ',false);
    RFileIO::writeAscii(outFile,line.sectionModulusY,false);
    RFileIO::writeAscii(outFile,' ',false);
    RFileIO::writeAscii(outFile,line.sectionModulusZ,false);
    RFileIO::writeAscii(outFile,' ',false);
    RFileIO::writeAscii(outFile,line.torsionalSectionModulus,addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RLine &line)
{
    RFileIO::writeBinary(outFile,static_cast<const RElementGroup&>(line));
    RFileIO::writeBinary(outFile,line.crossArea);
    RFileIO::writeBinary(outFile,line.secondMomentY);
    RFileIO::writeBinary(outFile,line.secondMomentZ);
    RFileIO::writeBinary(outFile,line.torsionalConstant);
    RFileIO::writeBinary(outFile,line.localYDirection);
    RFileIO::writeBinary(outFile,line.sectionModulusY);
    RFileIO::writeBinary(outFile,line.sectionModulusZ);
    RFileIO::writeBinary(outFile,line.torsionalSectionModulus);
}


/*********************************************************************
 *  RSurface                                                         *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RSurface &surface)
{
    RFileIO::readAscii(inFile,static_cast<RElementGroup&>(surface));
    RFileIO::readAscii(inFile,surface.thickness);
    if (inFile.getVersion() > RVersion(1,3,0))
    {
        unsigned int formulation = 0;
        RFileIO::readAscii(inFile,formulation);
        surface.setStructuralFormulation(static_cast<RElementFormulation>(formulation));
    }
}

void RFileIO::readBinary(RFile &inFile, RSurface &surface)
{
    RFileIO::readBinary(inFile,static_cast<RElementGroup&>(surface));
    RFileIO::readBinary(inFile,surface.thickness);
    if (inFile.getVersion() > RVersion(1,3,0))
    {
        unsigned int formulation = 0;
        RFileIO::readBinary(inFile,formulation);
        surface.setStructuralFormulation(static_cast<RElementFormulation>(formulation));
    }
}

void RFileIO::writeAscii(RSaveFile &outFile, const RSurface &surface, bool addNewLine)
{
    RFileIO::writeAscii(outFile,static_cast<const RElementGroup&>(surface),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,surface.thickness,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,static_cast<unsigned int>(surface.structuralFormulation),addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RSurface &surface)
{
    RFileIO::writeBinary(outFile,static_cast<const RElementGroup&>(surface));
    RFileIO::writeBinary(outFile,surface.thickness);
    RFileIO::writeBinary(outFile,static_cast<unsigned int>(surface.structuralFormulation));
}


/*********************************************************************
 *  RVolume                                                          *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RVolume &volume)
{
    RFileIO::readAscii(inFile,static_cast<RElementGroup&>(volume));
}

void RFileIO::readBinary(RFile &inFile, RVolume &volume)
{
    RFileIO::readBinary(inFile,static_cast<RElementGroup&>(volume));
}

void RFileIO::writeAscii(RSaveFile &outFile, const RVolume &volume, bool addNewLine)
{
    RFileIO::writeAscii(outFile,static_cast<const RElementGroup&>(volume),addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RVolume &volume)
{
    RFileIO::writeBinary(outFile,static_cast<const RElementGroup&>(volume));
}


/*********************************************************************
 *  RVectorField                                                     *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RVectorField &vectorField)
{
    RFileIO::readAscii(inFile,static_cast<REntityGroup&>(vectorField));
    RFileIO::readAscii(inFile,vectorField.variableType);
    if (inFile.getVersion() > RVersion(0,3,3))
    {
        RFileIO::readAscii(inFile,vectorField.type3d);
    }
    unsigned int nElementGroupIDs;
    RFileIO::readAscii(inFile,nElementGroupIDs);
    vectorField.elementGroupIDs.resize(nElementGroupIDs);
    for (unsigned int i=0;i<nElementGroupIDs;i++)
    {
        RFileIO::readAscii(inFile,vectorField.elementGroupIDs[i]);
    }
}


void RFileIO::readBinary(RFile &inFile, RVectorField &vectorField)
{
    RFileIO::readBinary(inFile,static_cast<REntityGroup&>(vectorField));
    RFileIO::readBinary(inFile,vectorField.variableType);
    if (inFile.getVersion() > RVersion(0,3,3))
    {
        RFileIO::readBinary(inFile,vectorField.type3d);
    }
    unsigned int nElementGroupIDs;
    RFileIO::readBinary(inFile,nElementGroupIDs);
    vectorField.elementGroupIDs.resize(nElementGroupIDs);
    for (unsigned int i=0;i<nElementGroupIDs;i++)
    {
        RFileIO::readBinary(inFile,vectorField.elementGroupIDs[i]);
    }
}


void RFileIO::writeAscii(RSaveFile &outFile, const RVectorField &vectorField, bool addNewLine)
{
    RFileIO::writeAscii(outFile,static_cast<const REntityGroup&>(vectorField),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,vectorField.variableType,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,vectorField.type3d,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,(unsigned int)vectorField.elementGroupIDs.size(),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    for (unsigned int i=0;i<vectorField.elementGroupIDs.size();i++)
    {
        RFileIO::writeAscii(outFile,vectorField.elementGroupIDs[i],addNewLine);
        if (!addNewLine && i < vectorField.elementGroupIDs.size()-1)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
}


void RFileIO::writeBinary(RSaveFile &outFile, const RVectorField &vectorField)
{
    RFileIO::writeBinary(outFile,static_cast<const REntityGroup&>(vectorField));
    RFileIO::writeBinary(outFile,vectorField.variableType);
    RFileIO::writeBinary(outFile,vectorField.type3d);
    RFileIO::writeBinary(outFile,(unsigned int)vectorField.elementGroupIDs.size());
    for (unsigned int i=0;i<vectorField.elementGroupIDs.size();i++)
    {
        RFileIO::writeBinary(outFile,vectorField.elementGroupIDs[i]);
    }
}


/*********************************************************************
 *  RScalarField                                                   *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RScalarField &scalarField)
{
    RFileIO::readAscii(inFile,static_cast<RVectorField&>(scalarField));
}

void RFileIO::readBinary(RFile &inFile, RScalarField &scalarField)
{
    RFileIO::readBinary(inFile,static_cast<RVectorField&>(scalarField));
}

void RFileIO::writeAscii(RSaveFile &outFile, const RScalarField &scalarField, bool addNewLine)
{
    RFileIO::writeAscii(outFile,static_cast<const RVectorField&>(scalarField),addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RScalarField &scalarField)
{
    RFileIO::writeBinary(outFile,static_cast<const RVectorField&>(scalarField));
}


/*********************************************************************
 *  RStreamLine                                                      *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RStreamLine &streamLine)
{
    RFileIO::readAscii(inFile,static_cast<REntityGroup&>(streamLine));
    RFileIO::readAscii(inFile,streamLine.variableType);
    RFileIO::readAscii(inFile,streamLine.position);
}


void RFileIO::readBinary(RFile &inFile, RStreamLine &streamLine)
{
    RFileIO::readBinary(inFile,static_cast<REntityGroup&>(streamLine));
    RFileIO::readBinary(inFile,streamLine.variableType);
    RFileIO::readBinary(inFile,streamLine.position);
}


void RFileIO::writeAscii(RSaveFile &outFile, const RStreamLine &streamLine, bool addNewLine)
{
    RFileIO::writeAscii(outFile,static_cast<const REntityGroup&>(streamLine),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,streamLine.variableType,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,streamLine.position,addNewLine);
}


void RFileIO::writeBinary(RSaveFile &outFile, const RStreamLine &streamLine)
{
    RFileIO::writeBinary(outFile,static_cast<const REntityGroup&>(streamLine));
    RFileIO::writeBinary(outFile,streamLine.variableType);
    RFileIO::writeBinary(outFile,streamLine.position);
}


/*********************************************************************
 *  RCut                                                             *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RCut &cut)
{
    RFileIO::readAscii(inFile,static_cast<REntityGroup&>(cut));
    RFileIO::readAscii(inFile,cut.plane);
    unsigned int nElementGroupIDs;
    RFileIO::readAscii(inFile,nElementGroupIDs);
    cut.elementGroupIDs.resize(nElementGroupIDs);
    for (unsigned int i=0;i<nElementGroupIDs;i++)
    {
        RFileIO::readAscii(inFile,cut.elementGroupIDs[i]);
    }
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RCut &cut)
{
    RFileIO::readBinary(inFile,static_cast<REntityGroup&>(cut));
    RFileIO::readBinary(inFile,cut.plane);
    unsigned int nElementGroupIDs;
    RFileIO::readBinary(inFile,nElementGroupIDs);
    cut.elementGroupIDs.resize(nElementGroupIDs);
    for (unsigned int i=0;i<nElementGroupIDs;i++)
    {
        RFileIO::readBinary(inFile,cut.elementGroupIDs[i]);
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RCut &cut, bool addNewLine)
{
    RFileIO::writeAscii(outFile,static_cast<const REntityGroup&>(cut),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,cut.plane,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,(unsigned int)cut.elementGroupIDs.size(),addNewLine);
    for (unsigned int i=0;i<cut.elementGroupIDs.size();i++)
    {
        RFileIO::writeAscii(outFile,cut.elementGroupIDs[i],addNewLine);
        if (!addNewLine && i < cut.elementGroupIDs.size()-1)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RCut &cut)
{
    RFileIO::writeBinary(outFile,static_cast<const REntityGroup&>(cut));
    RFileIO::writeBinary(outFile,cut.plane);
    RFileIO::writeBinary(outFile,(unsigned int)cut.elementGroupIDs.size());
    for (unsigned int i=0;i<cut.elementGroupIDs.size();i++)
    {
        RFileIO::writeBinary(outFile,cut.elementGroupIDs[i]);
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RIso                                                             *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RIso &iso)
{
    RFileIO::readAscii(inFile,static_cast<REntityGroup&>(iso));
    RFileIO::readAscii(inFile,iso.variableType);
    RFileIO::readAscii(inFile,iso.variableValue);
    unsigned int nElementGroupIDs;
    RFileIO::readAscii(inFile,nElementGroupIDs);
    iso.elementGroupIDs.resize(nElementGroupIDs);
    for (unsigned int i=0;i<nElementGroupIDs;i++)
    {
        RFileIO::readAscii(inFile,iso.elementGroupIDs[i]);
    }
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RIso &iso)
{
    RFileIO::readBinary(inFile,static_cast<REntityGroup&>(iso));
    RFileIO::readBinary(inFile,iso.variableType);
    RFileIO::readBinary(inFile,iso.variableValue);
    unsigned int nElementGroupIDs;
    RFileIO::readBinary(inFile,nElementGroupIDs);
    iso.elementGroupIDs.resize(nElementGroupIDs);
    for (unsigned int i=0;i<nElementGroupIDs;i++)
    {
        RFileIO::readBinary(inFile,iso.elementGroupIDs[i]);
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RIso &iso, bool addNewLine)
{
    RFileIO::writeAscii(outFile,static_cast<const REntityGroup&>(iso),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,iso.variableType,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,iso.variableValue,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,(unsigned int)iso.elementGroupIDs.size(),addNewLine);
    for (unsigned int i=0;i<iso.elementGroupIDs.size();i++)
    {
        RFileIO::writeAscii(outFile,iso.elementGroupIDs[i],addNewLine);
        if (!addNewLine && i < iso.elementGroupIDs.size()-1)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RIso &iso)
{
    RFileIO::writeBinary(outFile,static_cast<const REntityGroup&>(iso));
    RFileIO::writeBinary(outFile,iso.variableType);
    RFileIO::writeBinary(outFile,iso.variableValue);
    RFileIO::writeBinary(outFile,(unsigned int)iso.elementGroupIDs.size());
    for (unsigned int i=0;i<iso.elementGroupIDs.size();i++)
    {
        RFileIO::writeBinary(outFile,iso.elementGroupIDs[i]);
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RFileType                                                        *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RFileType &fileType)
{
    int iValue;
    inFile.getTextStream() >> iValue;
    if (inFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF, "Failed to read RFileType value.");
    }
    fileType = RFileType(iValue);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RFileType &fileType)
{
    int iValue;
    RFileIO::readBinary(inFile,iValue);
    fileType = RFileType(iValue);
//    inFile.read((char*)&fileType,sizeof(RFileType));
//    if (inFile.error() != RFile::NoError)
//    {
//        throw RError(RError::Type::ReadFile,R_ERROR_REF,"Failed to read RFileType value.");
//    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RFileType &fileType, bool addNewLine)
{
    if (!addNewLine)
    {
        outFile.getTextStream() << int(fileType);
    }
    else
    {
        outFile.getTextStream() << int(fileType) << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write RFileType value.");
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RFileType &fileType)
{
    RFileIO::writeBinary(outFile,int(fileType));
//    outFile.write((char*)&fileType,sizeof(RFileType));
//    if (outFile.error() != RFile::NoError)
//    {
//        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write RFileType value.");
//    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RFileHeader                                                      *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RFileHeader &fileHeader)
{
    RFileIO::readAscii(inFile,fileHeader.version);
    RFileIO::readAscii(inFile,fileHeader.type);
    RFileIO::readAscii(inFile,fileHeader.information);
}

void RFileIO::readBinary(RFile &inFile, RFileHeader &fileHeader)
{
    RFileIO::readBinary(inFile,fileHeader.version);
    RFileIO::readBinary(inFile,fileHeader.type);
    RFileIO::readBinary(inFile,fileHeader.information);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RFileHeader &fileHeader, bool addNewLine)
{
    outFile.setVersion(fileHeader.getVersion());
    RFileIO::writeAscii(outFile,fileHeader.version,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,fileHeader.type,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,fileHeader.information,addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RFileHeader &fileHeader)
{
    outFile.setVersion(fileHeader.getVersion());
    RFileIO::writeBinary(outFile,fileHeader.version);
    RFileIO::writeBinary(outFile,fileHeader.type);
    RFileIO::writeBinary(outFile,fileHeader.information);
}


/*********************************************************************
 *  RTimeMarchApproximation                                          *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RTimeMarchApproximation &timeMarchApproximation)
{
    int iValue;
    inFile.getTextStream() >> iValue;
    if (inFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF, "Failed to read RTimeMarchApproximation value.");
    }
    timeMarchApproximation = RTimeMarchApproximation(iValue);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RTimeMarchApproximation &timeMarchApproximation)
{
    inFile.read((char*)&timeMarchApproximation,sizeof(RTimeMarchApproximation));
    if (inFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,"Failed to read RTimeMarchApproximation value.");
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RTimeMarchApproximation &timeMarchApproximation, bool addNewLine)
{
    if (!addNewLine)
    {
        outFile.getTextStream() << int(timeMarchApproximation);
    }
    else
    {
        outFile.getTextStream() << int(timeMarchApproximation) << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write RTimeMarchApproximation value.");
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RTimeMarchApproximation &timeMarchApproximation)
{
    outFile.write((char*)&timeMarchApproximation,sizeof(RTimeMarchApproximation));
    if (outFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write RTimeMarchApproximation value.");
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RModalMethod                                                     *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RModalMethod &method)
{
    int iValue;
    inFile.getTextStream() >> iValue;
    if (inFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF, "Failed to read RModalMethod value.");
    }
    method = RModalMethod(iValue);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RModalMethod &method)
{
    inFile.read((char*)&method,sizeof(RModalMethod));
    if (inFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,"Failed to read RModalMethod value.");
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RModalMethod &method, bool addNewLine)
{
    if (!addNewLine)
    {
        outFile.getTextStream() << int(method);
    }
    else
    {
        outFile.getTextStream() << int(method) << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write RModalMethod value.");
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RModalMethod &method)
{
    outFile.write((char*)&method,sizeof(RModalMethod));
    if (outFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write RModalMethod value.");
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RAcousticAnalysisType                                            *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RAcousticAnalysisType &acousticAnalysisType)
{
    int iValue;
    inFile.getTextStream() >> iValue;
    if (inFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF, "Failed to read RAcousticAnalysisType value.");
    }
    acousticAnalysisType = RAcousticAnalysisType(iValue);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RAcousticAnalysisType &acousticAnalysisType)
{
    inFile.read((char*)&acousticAnalysisType,sizeof(RAcousticAnalysisType));
    if (inFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,"Failed to read RAcousticAnalysisType value.");
    }
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RAcousticAnalysisType &acousticAnalysisType, bool addNewLine)
{
    if (!addNewLine)
    {
        outFile.getTextStream() << int(acousticAnalysisType);
    }
    else
    {
        outFile.getTextStream() << int(acousticAnalysisType) << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write RAcousticAnalysisType value.");
    }
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RAcousticAnalysisType &acousticAnalysisType)
{
    outFile.write((char*)&acousticAnalysisType,sizeof(RAcousticAnalysisType));
    if (outFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write RAcousticAnalysisType value.");
    }
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RAcousticSetup                                                   *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, RAcousticSetup &acousticSetup)
{
    RFileIO::readAscii(inFile,acousticSetup.analysisType);
    RFileIO::readAscii(inFile,acousticSetup.frequencyStart);
    RFileIO::readAscii(inFile,acousticSetup.frequencyStep);
    RFileIO::readAscii(inFile,acousticSetup.nFrequencies);
    RFileIO::readAscii(inFile,acousticSetup.referencePressure);
    RFileIO::readAscii(inFile,acousticSetup.frequencyIndex);
}

void RFileIO::readBinary(RFile &inFile, RAcousticSetup &acousticSetup)
{
    RFileIO::readBinary(inFile,acousticSetup.analysisType);
    RFileIO::readBinary(inFile,acousticSetup.frequencyStart);
    RFileIO::readBinary(inFile,acousticSetup.frequencyStep);
    RFileIO::readBinary(inFile,acousticSetup.nFrequencies);
    RFileIO::readBinary(inFile,acousticSetup.referencePressure);
    RFileIO::readBinary(inFile,acousticSetup.frequencyIndex);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RAcousticSetup &acousticSetup, bool addNewLine)
{
    RFileIO::writeAscii(outFile,acousticSetup.analysisType,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,acousticSetup.frequencyStart,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,acousticSetup.frequencyStep,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,acousticSetup.nFrequencies,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,acousticSetup.referencePressure,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,acousticSetup.frequencyIndex,addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RAcousticSetup &acousticSetup)
{
    RFileIO::writeBinary(outFile,acousticSetup.analysisType);
    RFileIO::writeBinary(outFile,acousticSetup.frequencyStart);
    RFileIO::writeBinary(outFile,acousticSetup.frequencyStep);
    RFileIO::writeBinary(outFile,acousticSetup.nFrequencies);
    RFileIO::writeBinary(outFile,acousticSetup.referencePressure);
    RFileIO::writeBinary(outFile,acousticSetup.frequencyIndex);
}


/*********************************************************************
 *  RTimeSolver                                                      *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RTimeSolver &timeSolver)
{
    RFileIO::readAscii(inFile,timeSolver.enabled);
    RFileIO::readAscii(inFile,timeSolver.timeMarchApproximation);
    unsigned int nTimes = 0;
    RFileIO::readAscii(inFile,nTimes);
    timeSolver.times.resize(nTimes);
    for (unsigned int i=0;i<timeSolver.times.size();i++)
    {
        RFileIO::readAscii(inFile,timeSolver.times[i]);
    }
    RFileIO::readAscii(inFile,timeSolver.inputNTimeSteps);
    RFileIO::readAscii(inFile,timeSolver.inputStartTime);
    RFileIO::readAscii(inFile,timeSolver.inputTimeStepSize);
    RFileIO::readAscii(inFile,timeSolver.currentTimeStep);
    RFileIO::readAscii(inFile,timeSolver.computedTime);
    RFileIO::readAscii(inFile,timeSolver.outputFrequency);
}

void RFileIO::readBinary(RFile &inFile, RTimeSolver &timeSolver)
{
    RFileIO::readBinary(inFile,timeSolver.enabled);
    RFileIO::readBinary(inFile,timeSolver.timeMarchApproximation);
    unsigned int nTimes = 0;
    RFileIO::readBinary(inFile,nTimes);
    timeSolver.times.resize(nTimes);
    for (unsigned int i=0;i<timeSolver.times.size();i++)
    {
        RFileIO::readBinary(inFile,timeSolver.times[i]);
    }
    RFileIO::readBinary(inFile,timeSolver.inputNTimeSteps);
    RFileIO::readBinary(inFile,timeSolver.inputStartTime);
    RFileIO::readBinary(inFile,timeSolver.inputTimeStepSize);
    RFileIO::readBinary(inFile,timeSolver.currentTimeStep);
    RFileIO::readBinary(inFile,timeSolver.computedTime);
    RFileIO::readBinary(inFile,timeSolver.outputFrequency);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RTimeSolver &timeSolver, bool addNewLine)
{
    RFileIO::writeAscii(outFile,timeSolver.enabled,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,timeSolver.timeMarchApproximation,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,(unsigned int)timeSolver.times.size(),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    for (unsigned int i=0;i<timeSolver.times.size();i++)
    {
        RFileIO::writeAscii(outFile,timeSolver.times[i],addNewLine);
        if (!addNewLine)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
    RFileIO::writeAscii(outFile,timeSolver.inputNTimeSteps,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,timeSolver.inputStartTime,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,timeSolver.inputTimeStepSize,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,timeSolver.currentTimeStep,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,timeSolver.computedTime,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,timeSolver.outputFrequency,addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RTimeSolver &timeSolver)
{
    RFileIO::writeBinary(outFile,timeSolver.enabled);
    RFileIO::writeBinary(outFile,timeSolver.timeMarchApproximation);
    RFileIO::writeBinary(outFile,(unsigned int)timeSolver.times.size());
    for (unsigned int i=0;i<timeSolver.times.size();i++)
    {
        RFileIO::writeBinary(outFile,timeSolver.times[i]);
    }
    RFileIO::writeBinary(outFile,timeSolver.inputNTimeSteps);
    RFileIO::writeBinary(outFile,timeSolver.inputStartTime);
    RFileIO::writeBinary(outFile,timeSolver.inputTimeStepSize);
    RFileIO::writeBinary(outFile,timeSolver.currentTimeStep);
    RFileIO::writeBinary(outFile,timeSolver.computedTime);
    RFileIO::writeBinary(outFile,timeSolver.outputFrequency);
}


/*********************************************************************
 *  RMatrixSolverConf                                                *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RMatrixSolverConf &matrixSolver)
{
    RFileIO::readAscii(inFile,matrixSolver.type);
    RFileIO::readAscii(inFile,matrixSolver.nInnerIterations);
    RFileIO::readAscii(inFile,matrixSolver.nOuterIterations);
    RFileIO::readAscii(inFile,matrixSolver.solverCvgValue);
    RFileIO::readAscii(inFile,matrixSolver.outputFrequency);
} /* RFileIO::readAscii */


void RFileIO::readBinary(RFile &inFile, RMatrixSolverConf &matrixSolver)
{
    RFileIO::readBinary(inFile,matrixSolver.type);
    RFileIO::readBinary(inFile,matrixSolver.nInnerIterations);
    RFileIO::readBinary(inFile,matrixSolver.nOuterIterations);
    RFileIO::readBinary(inFile,matrixSolver.solverCvgValue);
    RFileIO::readBinary(inFile,matrixSolver.outputFrequency);
} /* RFileIO::readBinary */


void RFileIO::writeAscii(RSaveFile &outFile, const RMatrixSolverConf &matrixSolver, bool addNewLine)
{
    RFileIO::writeAscii(outFile,matrixSolver.type,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,matrixSolver.nInnerIterations,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,matrixSolver.nOuterIterations,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,matrixSolver.solverCvgValue,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,matrixSolver.outputFrequency,addNewLine);
} /* RFileIO::writeAscii */


void RFileIO::writeBinary(RSaveFile &outFile, const RMatrixSolverConf &matrixSolver)
{
    RFileIO::writeBinary(outFile,matrixSolver.type);
    RFileIO::writeBinary(outFile,matrixSolver.nInnerIterations);
    RFileIO::writeBinary(outFile,matrixSolver.nOuterIterations);
    RFileIO::writeBinary(outFile,matrixSolver.solverCvgValue);
    RFileIO::writeBinary(outFile,matrixSolver.outputFrequency);
} /* RFileIO::writeBinary */


/*********************************************************************
 *  RMonitoringPoint                                                 *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RMonitoringPoint &monitoringPoint)
{
    RFileIO::readAscii(inFile,monitoringPoint.variableType);
    RFileIO::readAscii(inFile,monitoringPoint.position);
}


void RFileIO::readBinary(RFile &inFile, RMonitoringPoint &monitoringPoint)
{
    RFileIO::readBinary(inFile,monitoringPoint.variableType);
    RFileIO::readBinary(inFile,monitoringPoint.position);
}


void RFileIO::writeAscii(RSaveFile &outFile, const RMonitoringPoint &monitoringPoint, bool addNewLine)
{
    RFileIO::writeAscii(outFile,monitoringPoint.variableType,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,monitoringPoint.position,false,addNewLine);
}


void RFileIO::writeAscii(RFile &outFile, const RMonitoringPoint &monitoringPoint, bool addNewLine)
{
    RFileIO::writeAscii(outFile,monitoringPoint.variableType,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,monitoringPoint.position,false,addNewLine);
}


void RFileIO::writeBinary(RSaveFile &outFile, const RMonitoringPoint &monitoringPoint)
{
    RFileIO::writeBinary(outFile,monitoringPoint.variableType);
    RFileIO::writeBinary(outFile,monitoringPoint.position);
}


/*********************************************************************
 *  RMonitoringPointManager                                          *
 *********************************************************************/


void RFileIO::readAscii(RFile &inFile, RMonitoringPointManager &monitoringPointManager)
{
    unsigned int nMonitoringPoints;
    RFileIO::readAscii(inFile,nMonitoringPoints);
    monitoringPointManager.resize(nMonitoringPoints);
    for (unsigned int i=0;i<nMonitoringPoints;i++)
    {
        RFileIO::readAscii(inFile,monitoringPointManager[i]);
    }
}


void RFileIO::readBinary(RFile &inFile, RMonitoringPointManager &monitoringPointManager)
{
    unsigned int nMonitoringPoints;
    RFileIO::readBinary(inFile,nMonitoringPoints);
    monitoringPointManager.resize(nMonitoringPoints);
    for (unsigned int i=0;i<nMonitoringPoints;i++)
    {
        RFileIO::readBinary(inFile,monitoringPointManager[i]);
    }
}


void RFileIO::writeAscii(RSaveFile &outFile, const RMonitoringPointManager &monitoringPointManager, bool addNewLine)
{
    RFileIO::writeAscii(outFile,(unsigned int)monitoringPointManager.size(),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    for (unsigned int i=0;i<monitoringPointManager.size();i++)
    {
        RFileIO::writeAscii(outFile,monitoringPointManager[i],addNewLine);
        if (i < monitoringPointManager.size()-1 && !addNewLine)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
}


void RFileIO::writeBinary(RSaveFile &outFile, const RMonitoringPointManager &monitoringPointManager)
{
    RFileIO::writeBinary(outFile,(unsigned int)monitoringPointManager.size());
    for (unsigned int i=0;i<monitoringPointManager.size();i++)
    {
        RFileIO::writeBinary(outFile,monitoringPointManager[i]);
    }
}


/*********************************************************************
 *  RProblemType                                                     *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, RProblemType &problemType)
{
    QString id;
    RFileIO::readAscii(inFile,id);
    problemType = RProblem::getTypeFromId(id);
}

void RFileIO::readBinary(RFile &inFile, RProblemType &problemType)
{
    QString id;
    RFileIO::readBinary(inFile,id);
    problemType = RProblem::getTypeFromId(id);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RProblemType &problemType, bool addNewLine)
{
    RFileIO::writeAscii(outFile,RProblem::getId(problemType),addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RProblemType &problemType)
{
    RFileIO::writeBinary(outFile,RProblem::getId(problemType));
}


/*********************************************************************
 *  RProblemTaskItem                                                 *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, RProblemTaskItem &problemTaskItem)
{
    RFileIO::readAscii(inFile,problemTaskItem.problemType);
    RFileIO::readAscii(inFile,problemTaskItem.nIterations);
    uint nChildren = 0;
    RFileIO::readAscii(inFile,nChildren);
    for (uint i=0;i<nChildren;i++)
    {
        RProblemTaskItem child;
        RFileIO::readAscii(inFile,child);
        problemTaskItem.addChild(child);
    }
}

void RFileIO::readBinary(RFile &inFile, RProblemTaskItem &problemTaskItem)
{
    RFileIO::readBinary(inFile,problemTaskItem.problemType);
    RFileIO::readBinary(inFile,problemTaskItem.nIterations);
    uint nChildren = 0;
    RFileIO::readBinary(inFile,nChildren);
    for (uint i=0;i<nChildren;i++)
    {
        RProblemTaskItem child;
        RFileIO::readBinary(inFile,child);
        problemTaskItem.addChild(child);
    }
}

void RFileIO::writeAscii(RSaveFile &outFile, const RProblemTaskItem &problemTaskItem, bool addNewLine)
{
    RFileIO::writeAscii(outFile,problemTaskItem.problemType,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,problemTaskItem.nIterations,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,uint(problemTaskItem.children.size()),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    for (uint i=0;i<problemTaskItem.children.size();i++)
    {
        RFileIO::writeAscii(outFile,problemTaskItem.children[i],addNewLine);
        if (i < problemTaskItem.children.size()-1 && !addNewLine)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
}

void RFileIO::writeBinary(RSaveFile &outFile, const RProblemTaskItem &problemTaskItem)
{
    RFileIO::writeBinary(outFile,problemTaskItem.problemType);
    RFileIO::writeBinary(outFile,problemTaskItem.nIterations);
    RFileIO::writeBinary(outFile,uint(problemTaskItem.children.size()));
    for (uint i=0;i<problemTaskItem.children.size();i++)
    {
        RFileIO::writeBinary(outFile,problemTaskItem.children[i]);
    }
}


/*********************************************************************
 *  RProblemSetup                                                    *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, RProblemSetup &problemSetup)
{
    RProblemSetup candidate(problemSetup);
    RFileIO::readAscii(inFile,candidate.restart);
    RFileIO::readAscii(inFile,candidate.radiationSetup);
    RFileIO::readAscii(inFile,candidate.modalSetup);
    if (inFile.getVersion() > RVersion(0,3,4))
    {
        RFileIO::readAscii(inFile,candidate.meshSetup);
    }
    if (inFile.getVersion() > RVersion(1,2,0))
    {
        RFileIO::readAscii(inFile,candidate.acousticSetup);
    }
    if (inFile.getVersion() > RVersion(1,3,1))
    {
        int massMatrixType;
        RFileIO::readAscii(inFile,massMatrixType);
        candidate.setMassMatrixType(static_cast<RMassMatrixType>(massMatrixType));
    }
    if (inFile.getVersion() > RVersion(1,3,4))
    {
        RFileIO::readAscii(inFile,candidate.structuralInteractionSetup);
    }
    if (inFile.getVersion() > RVersion(1,3,5))
    {
        RFileIO::readAscii(inFile,candidate.structuralDynamicsSetup);
        RFileIO::readAscii(inFile,candidate.parallelExecutionConfig);
    }
    else
    {
        candidate.structuralDynamicsSetup = RStructuralDynamicsSetup();
        candidate.parallelExecutionConfig = RParallelExecutionConfig();
    }
    if (inFile.getVersion() > RVersion(1,3,6))
    {
        RFileIO::readAscii(inFile,candidate.structuralDynamicsRestartData);
    }
    else
    {
        candidate.structuralDynamicsRestartData.clear();
    }
    if (inFile.getVersion() > RVersion(1,3,10))
        RFileIO::readAscii(inFile,candidate.structuralControlSetup);
    else
        candidate.structuralControlSetup = RStructuralControlSetup();
    if (inFile.getVersion() > RVersion(1,3,12))
        RFileIO::readAscii(inFile,candidate.structuralCableSetup);
    else
        candidate.structuralCableSetup = RStructuralCableSetup();
    if (inFile.getVersion() > RVersion(1,3,13))
        RFileIO::readAscii(inFile,candidate.structuralPiezoSetup);
    else
        candidate.structuralPiezoSetup = RStructuralPiezoSetup();
    if (inFile.getVersion() > RVersion(1,3,15))
        RFileIO::readAscii(inFile,candidate.structuralThermalControlSetup);
    else
        candidate.structuralThermalControlSetup = RStructuralThermalControlSetup();
    if (inFile.getVersion() > RVersion(1,3,19))
    {
        RFileIO::readAscii(inFile,candidate.structuralTrussDynamicsSetup);
        RFileIO::readAscii(inFile,candidate.structuralTrussDynamicsRestartData);
    }
    else
    {
        candidate.structuralTrussDynamicsSetup = RStructuralTrussDynamicsSetup();
        candidate.structuralTrussDynamicsRestartData.clear();
    }
    if (inFile.getVersion() > RVersion(1,3,20))
    {
        RFileIO::readAscii(inFile,candidate.structuralNativeControlSpec);
        RFileIO::readAscii(inFile,candidate.structuralNativeControlState);
        candidate.structuralNativeControlState.validate(candidate.structuralNativeControlSpec);
        if (candidate.structuralNativeControlSpec.source == RStructuralNativeControlSource::LocalV3
            && (!candidate.structuralTrussDynamicsSetup.enabled || !candidate.structuralTrussDynamicsSetup.heldCommands.empty()))
            throw RError(RError::Type::ReadFile,R_ERROR_REF,"Local native control requires enabled trusses and an empty frozen schedule.");
        if (candidate.structuralNativeControlState.available && candidate.structuralNativeControlState.sourceFingerprint != candidate.structuralTrussDynamicsSetup.sourceFingerprint)
            throw RError(RError::Type::ReadFile,R_ERROR_REF,"Native control state source differs from its truss source.");
    }
    else
    {
        candidate.structuralNativeControlSpec = RStructuralNativeControlSpec();
        candidate.structuralNativeControlState.clear();
    }
    problemSetup = candidate;
}

void RFileIO::readBinary(RFile &inFile, RProblemSetup &problemSetup)
{
    RProblemSetup candidate(problemSetup);
    RFileIO::readBinary(inFile,candidate.restart);
    RFileIO::readBinary(inFile,candidate.radiationSetup);
    RFileIO::readBinary(inFile,candidate.modalSetup);
    if (inFile.getVersion() > RVersion(0,3,4))
    {
        RFileIO::readBinary(inFile,candidate.meshSetup);
    }
    if (inFile.getVersion() > RVersion(1,2,0))
    {
        RFileIO::readBinary(inFile,candidate.acousticSetup);
    }
    if (inFile.getVersion() > RVersion(1,3,1))
    {
        int massMatrixType;
        RFileIO::readBinary(inFile,massMatrixType);
        candidate.setMassMatrixType(static_cast<RMassMatrixType>(massMatrixType));
    }
    if (inFile.getVersion() > RVersion(1,3,4))
    {
        RFileIO::readBinary(inFile,candidate.structuralInteractionSetup);
    }
    if (inFile.getVersion() > RVersion(1,3,5))
    {
        RFileIO::readBinary(inFile,candidate.structuralDynamicsSetup);
        RFileIO::readBinary(inFile,candidate.parallelExecutionConfig);
    }
    else
    {
        candidate.structuralDynamicsSetup = RStructuralDynamicsSetup();
        candidate.parallelExecutionConfig = RParallelExecutionConfig();
    }
    if (inFile.getVersion() > RVersion(1,3,6))
    {
        RFileIO::readBinary(inFile,candidate.structuralDynamicsRestartData);
    }
    else
    {
        candidate.structuralDynamicsRestartData.clear();
    }
    if (inFile.getVersion() > RVersion(1,3,10))
        RFileIO::readBinary(inFile,candidate.structuralControlSetup);
    else
        candidate.structuralControlSetup = RStructuralControlSetup();
    if (inFile.getVersion() > RVersion(1,3,12))
        RFileIO::readBinary(inFile,candidate.structuralCableSetup);
    else
        candidate.structuralCableSetup = RStructuralCableSetup();
    if (inFile.getVersion() > RVersion(1,3,13))
        RFileIO::readBinary(inFile,candidate.structuralPiezoSetup);
    else
        candidate.structuralPiezoSetup = RStructuralPiezoSetup();
    if (inFile.getVersion() > RVersion(1,3,15))
        RFileIO::readBinary(inFile,candidate.structuralThermalControlSetup);
    else
        candidate.structuralThermalControlSetup = RStructuralThermalControlSetup();
    if (inFile.getVersion() > RVersion(1,3,19))
    {
        RFileIO::readBinary(inFile,candidate.structuralTrussDynamicsSetup);
        RFileIO::readBinary(inFile,candidate.structuralTrussDynamicsRestartData);
    }
    else
    {
        candidate.structuralTrussDynamicsSetup = RStructuralTrussDynamicsSetup();
        candidate.structuralTrussDynamicsRestartData.clear();
    }
    if (inFile.getVersion() > RVersion(1,3,20))
    {
        RFileIO::readBinary(inFile,candidate.structuralNativeControlSpec);
        RFileIO::readBinary(inFile,candidate.structuralNativeControlState);
        candidate.structuralNativeControlState.validate(candidate.structuralNativeControlSpec);
        if (candidate.structuralNativeControlSpec.source == RStructuralNativeControlSource::LocalV3
            && (!candidate.structuralTrussDynamicsSetup.enabled || !candidate.structuralTrussDynamicsSetup.heldCommands.empty()))
            throw RError(RError::Type::ReadFile,R_ERROR_REF,"Local native control requires enabled trusses and an empty frozen schedule.");
        if (candidate.structuralNativeControlState.available && candidate.structuralNativeControlState.sourceFingerprint != candidate.structuralTrussDynamicsSetup.sourceFingerprint)
            throw RError(RError::Type::ReadFile,R_ERROR_REF,"Native control state source differs from its truss source.");
    }
    else
    {
        candidate.structuralNativeControlSpec = RStructuralNativeControlSpec();
        candidate.structuralNativeControlState.clear();
    }
    problemSetup = candidate;
}

void RFileIO::writeAscii(RSaveFile &outFile, const RProblemSetup &problemSetup, bool addNewLine)
{
    problemSetup.structuralNativeControlSpec.validate();
    problemSetup.structuralNativeControlState.validate(problemSetup.structuralNativeControlSpec);
    if (problemSetup.structuralNativeControlSpec.source == RStructuralNativeControlSource::LocalV3
        && (!problemSetup.structuralTrussDynamicsSetup.enabled || !problemSetup.structuralTrussDynamicsSetup.heldCommands.empty()))
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Local native control requires enabled trusses and an empty frozen schedule.");
    if (outFile.getVersion() <= RVersion(1,3,20)
        && (problemSetup.structuralNativeControlSpec.hasNonDefaultData() || problemSetup.structuralNativeControlState.hasNonDefaultData()))
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Native control v3 requires model format 1.3.21.");
    if (problemSetup.structuralNativeControlState.available && problemSetup.structuralNativeControlState.sourceFingerprint != problemSetup.structuralTrussDynamicsSetup.sourceFingerprint)
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Native control state source differs from its truss source.");
    problemSetup.structuralTrussDynamicsSetup.validate();
    problemSetup.structuralTrussDynamicsRestartData.validate();
    if (outFile.getVersion() <= RVersion(1,3,19)
        && (problemSetup.structuralTrussDynamicsSetup.hasNonDefaultData()
            || problemSetup.structuralTrussDynamicsRestartData.hasNonDefaultData()))
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Native mixed TRUSS1 data requires model format 1.3.20.");
    RFileIO::writeAscii(outFile,problemSetup.restart,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,problemSetup.radiationSetup,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,problemSetup.modalSetup,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,problemSetup.meshSetup,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,problemSetup.acousticSetup,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,int(problemSetup.massMatrixType),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,problemSetup.structuralInteractionSetup,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,problemSetup.structuralDynamicsSetup,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,problemSetup.parallelExecutionConfig,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,problemSetup.structuralDynamicsRestartData,addNewLine);
    if (!addNewLine) RFileIO::writeAscii(outFile,' ',false);
    RFileIO::writeAscii(outFile,problemSetup.structuralControlSetup,addNewLine);
    if (!addNewLine) RFileIO::writeAscii(outFile,' ',false);
    RFileIO::writeAscii(outFile,problemSetup.structuralCableSetup,addNewLine);
    if (!addNewLine) RFileIO::writeAscii(outFile,' ',false);
    RFileIO::writeAscii(outFile,problemSetup.structuralPiezoSetup,addNewLine);
    if (!addNewLine) RFileIO::writeAscii(outFile,' ',false);
    RFileIO::writeAscii(outFile,problemSetup.structuralThermalControlSetup,addNewLine);
    if (!addNewLine && outFile.getVersion() > RVersion(1,3,19)) RFileIO::writeAscii(outFile,' ',false);
    RFileIO::writeAscii(outFile,problemSetup.structuralTrussDynamicsSetup,addNewLine);
    if (!addNewLine && outFile.getVersion() > RVersion(1,3,19)) RFileIO::writeAscii(outFile,' ',false);
    RFileIO::writeAscii(outFile,problemSetup.structuralTrussDynamicsRestartData,addNewLine);
    if (!addNewLine && outFile.getVersion() > RVersion(1,3,20)) RFileIO::writeAscii(outFile,' ',false);
    RFileIO::writeAscii(outFile,problemSetup.structuralNativeControlSpec,addNewLine);
    if (!addNewLine && outFile.getVersion() > RVersion(1,3,20)) RFileIO::writeAscii(outFile,' ',false);
    RFileIO::writeAscii(outFile,problemSetup.structuralNativeControlState,addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RProblemSetup &problemSetup)
{
    problemSetup.structuralNativeControlSpec.validate();
    problemSetup.structuralNativeControlState.validate(problemSetup.structuralNativeControlSpec);
    if (problemSetup.structuralNativeControlSpec.source == RStructuralNativeControlSource::LocalV3
        && (!problemSetup.structuralTrussDynamicsSetup.enabled || !problemSetup.structuralTrussDynamicsSetup.heldCommands.empty()))
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Local native control requires enabled trusses and an empty frozen schedule.");
    if (outFile.getVersion() <= RVersion(1,3,20)
        && (problemSetup.structuralNativeControlSpec.hasNonDefaultData() || problemSetup.structuralNativeControlState.hasNonDefaultData()))
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Native control v3 requires model format 1.3.21.");
    if (problemSetup.structuralNativeControlState.available && problemSetup.structuralNativeControlState.sourceFingerprint != problemSetup.structuralTrussDynamicsSetup.sourceFingerprint)
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Native control state source differs from its truss source.");
    problemSetup.structuralTrussDynamicsSetup.validate();
    problemSetup.structuralTrussDynamicsRestartData.validate();
    if (outFile.getVersion() <= RVersion(1,3,19)
        && (problemSetup.structuralTrussDynamicsSetup.hasNonDefaultData()
            || problemSetup.structuralTrussDynamicsRestartData.hasNonDefaultData()))
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Native mixed TRUSS1 data requires model format 1.3.20.");
    RFileIO::writeBinary(outFile,problemSetup.restart);
    RFileIO::writeBinary(outFile,problemSetup.radiationSetup);
    RFileIO::writeBinary(outFile,problemSetup.modalSetup);
    RFileIO::writeBinary(outFile,problemSetup.meshSetup);
    RFileIO::writeBinary(outFile,problemSetup.acousticSetup);
    RFileIO::writeBinary(outFile,int(problemSetup.massMatrixType));
    RFileIO::writeBinary(outFile,problemSetup.structuralInteractionSetup);
    RFileIO::writeBinary(outFile,problemSetup.structuralDynamicsSetup);
    RFileIO::writeBinary(outFile,problemSetup.parallelExecutionConfig);
    RFileIO::writeBinary(outFile,problemSetup.structuralDynamicsRestartData);
    RFileIO::writeBinary(outFile,problemSetup.structuralControlSetup);
    RFileIO::writeBinary(outFile,problemSetup.structuralCableSetup);
    RFileIO::writeBinary(outFile,problemSetup.structuralPiezoSetup);
    RFileIO::writeBinary(outFile,problemSetup.structuralThermalControlSetup);
    RFileIO::writeBinary(outFile,problemSetup.structuralTrussDynamicsSetup);
    RFileIO::writeBinary(outFile,problemSetup.structuralTrussDynamicsRestartData);
    RFileIO::writeBinary(outFile,problemSetup.structuralNativeControlSpec);
    RFileIO::writeBinary(outFile,problemSetup.structuralNativeControlState);
}


/*********************************************************************
 *  Stage 5/6 transient dynamics and control configuration           *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, RStructuralControlSetup &setup)
{
    RStructuralControlSetup result;
    RFileIO::readAscii(inFile,result.enabled);
    RFileIO::readAscii(inFile,result.controlProtocolVersion);
    RFileIO::readAscii(inFile,result.controllerId);
    QString seed;
    RFileIO::readAscii(inFile,seed);
    bool ok = false;
    result.deterministicSeed = seed.toULongLong(&ok,10);
    if (!ok || seed != QString::number(result.deterministicSeed))
        throw RError(RError::Type::ReadFile,R_ERROR_REF,"Invalid structural control seed.");
    if (inFile.getVersion() > RVersion(1,3,11))
    {
        uint count = 0;
        RFileIO::readAscii(inFile,count);
        if (count > 4096u)
            throw RError(RError::Type::ReadFile,R_ERROR_REF,
                         "Invalid structural control conditioning channel count (%u).",count);
        result.channelConditioning.resize(count);
        for (auto &channel : result.channelConditioning)
        {
            RFileIO::readAscii(inFile,channel.actuatorId);
            RFileIO::readAscii(inFile,channel.delaySteps);
            RFileIO::readAscii(inFile,channel.holdSteps);
            RFileIO::readAscii(inFile,channel.noiseAmplitude);
        }
    }
    result.validate();
    setup = std::move(result);
}

void RFileIO::readBinary(RFile &inFile, RStructuralControlSetup &setup)
{
    RStructuralControlSetup result;
    RFileIO::readBinary(inFile,result.enabled);
    RFileIO::readBinary(inFile,result.controlProtocolVersion);
    RFileIO::readBinary(inFile,result.controllerId);
    uint low = 0, high = 0;
    RFileIO::readBinary(inFile,low);
    RFileIO::readBinary(inFile,high);
    result.deterministicSeed = (quint64(high) << 32) | low;
    if (inFile.getVersion() > RVersion(1,3,11))
    {
        uint count = 0;
        RFileIO::readBinary(inFile,count);
        if (count > 4096u)
            throw RError(RError::Type::ReadFile,R_ERROR_REF,
                         "Invalid structural control conditioning channel count (%u).",count);
        result.channelConditioning.resize(count);
        for (auto &channel : result.channelConditioning)
        {
            RFileIO::readBinary(inFile,channel.actuatorId);
            RFileIO::readBinary(inFile,channel.delaySteps);
            RFileIO::readBinary(inFile,channel.holdSteps);
            RFileIO::readBinary(inFile,channel.noiseAmplitude);
        }
    }
    result.validate();
    setup = std::move(result);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RStructuralControlSetup &setup, bool addNewLine)
{
    setup.validate();
    struct PrecisionGuard
    {
        QTextStream &stream;
        int previous;
        ~PrecisionGuard() { stream.setRealNumberPrecision(previous); }
    } precisionGuard{outFile.getTextStream(),outFile.getTextStream().realNumberPrecision()};
    outFile.getTextStream().setRealNumberPrecision(17);
    bool first = true;
    auto write = [&](const auto &value)
    {
        if (!first && !addNewLine) RFileIO::writeAscii(outFile,' ',false);
        RFileIO::writeAscii(outFile,value,addNewLine);
        first = false;
    };
    write(setup.enabled);
    write(setup.controlProtocolVersion);
    write(setup.controllerId);
    write(QString::number(setup.deterministicSeed));
    write(uint(setup.channelConditioning.size()));
    for (const auto &channel : setup.channelConditioning)
    {
        write(channel.actuatorId);
        write(channel.delaySteps);
        write(channel.holdSteps);
        write(channel.noiseAmplitude);
    }
}

void RFileIO::writeBinary(RSaveFile &outFile, const RStructuralControlSetup &setup)
{
    setup.validate();
    RFileIO::writeBinary(outFile,setup.enabled);
    RFileIO::writeBinary(outFile,setup.controlProtocolVersion);
    RFileIO::writeBinary(outFile,setup.controllerId);
    // Canonical two-word low/high order avoids platform-dependent uint64 ABI.
    RFileIO::writeBinary(outFile,uint(setup.deterministicSeed & 0xffffffffu));
    RFileIO::writeBinary(outFile,uint(setup.deterministicSeed >> 32));
    RFileIO::writeBinary(outFile,uint(setup.channelConditioning.size()));
    for (const auto &channel : setup.channelConditioning)
    {
        RFileIO::writeBinary(outFile,channel.actuatorId);
        RFileIO::writeBinary(outFile,channel.delaySteps);
        RFileIO::writeBinary(outFile,channel.holdSteps);
        RFileIO::writeBinary(outFile,channel.noiseAmplitude);
    }
}

/*********************************************************************
 * Version 1.3.16 multirate thermal-control configuration             *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, RStructuralThermalControlSetup &setup)
{
    RStructuralThermalControlSetup result;
    RFileIO::readAscii(inFile,result.enabled);
    RFileIO::readAscii(inFile,result.thermalSubstepSeconds);
    RFileIO::readAscii(inFile,result.thermalSubstepsPerMechanicalMacrostep);
    RFileIO::readAscii(inFile,result.mechanicalMacrostepsPerControllerSample);

    uint channelCount = 0;
    RFileIO::readAscii(inFile,channelCount);
    if (channelCount > 4096u)
        throw RError(RError::Type::ReadFile,R_ERROR_REF,
                     "Excessive thermal control channel count (%u).",channelCount);
    result.channels.resize(channelCount);
    for (auto &channel : result.channels)
    {
        RFileIO::readAscii(inFile,channel.actuatorId);
        RFileIO::readAscii(inFile,channel.heaterNodeId);
        RFileIO::readAscii(inFile,channel.temperatureSensorId);
        RFileIO::readAscii(inFile,channel.temperatureSensorNodeId);
        RFileIO::readAscii(inFile,channel.turnOnBelowKelvin);
        RFileIO::readAscii(inFile,channel.turnOffAtOrAboveKelvin);
        RFileIO::readAscii(inFile,channel.onPowerWatts);
        RFileIO::readAscii(inFile,channel.minimumPowerWatts);
        RFileIO::readAscii(inFile,channel.maximumPowerWatts);
        RFileIO::readAscii(inFile,channel.maximumPowerSlewWattsPerSecond);
        RFileIO::readAscii(inFile,channel.maximumPowerSlewUnlimited);
    }

    uint sparseNodeCount = 0;
    RFileIO::readAscii(inFile,sparseNodeCount);
    if (sparseNodeCount > 1000000u)
        throw RError(RError::Type::ReadFile,R_ERROR_REF,
                     "Excessive sparse thermal observation node count (%u).",sparseNodeCount);
    result.sparseObservationNodeIds.resize(sparseNodeCount);
    for (QString &nodeId : result.sparseObservationNodeIds)
        RFileIO::readAscii(inFile,nodeId);

    result.validate();
    setup = std::move(result);
}

void RFileIO::readBinary(RFile &inFile, RStructuralThermalControlSetup &setup)
{
    RStructuralThermalControlSetup result;
    RFileIO::readBinary(inFile,result.enabled);
    RFileIO::readBinary(inFile,result.thermalSubstepSeconds);
    RFileIO::readBinary(inFile,result.thermalSubstepsPerMechanicalMacrostep);
    RFileIO::readBinary(inFile,result.mechanicalMacrostepsPerControllerSample);

    uint channelCount = 0;
    RFileIO::readBinary(inFile,channelCount);
    if (channelCount > 4096u)
        throw RError(RError::Type::ReadFile,R_ERROR_REF,
                     "Excessive thermal control channel count (%u).",channelCount);
    result.channels.resize(channelCount);
    for (auto &channel : result.channels)
    {
        RFileIO::readBinary(inFile,channel.actuatorId);
        RFileIO::readBinary(inFile,channel.heaterNodeId);
        RFileIO::readBinary(inFile,channel.temperatureSensorId);
        RFileIO::readBinary(inFile,channel.temperatureSensorNodeId);
        RFileIO::readBinary(inFile,channel.turnOnBelowKelvin);
        RFileIO::readBinary(inFile,channel.turnOffAtOrAboveKelvin);
        RFileIO::readBinary(inFile,channel.onPowerWatts);
        RFileIO::readBinary(inFile,channel.minimumPowerWatts);
        RFileIO::readBinary(inFile,channel.maximumPowerWatts);
        RFileIO::readBinary(inFile,channel.maximumPowerSlewWattsPerSecond);
        RFileIO::readBinary(inFile,channel.maximumPowerSlewUnlimited);
    }

    uint sparseNodeCount = 0;
    RFileIO::readBinary(inFile,sparseNodeCount);
    if (sparseNodeCount > 1000000u)
        throw RError(RError::Type::ReadFile,R_ERROR_REF,
                     "Excessive sparse thermal observation node count (%u).",sparseNodeCount);
    result.sparseObservationNodeIds.resize(sparseNodeCount);
    for (QString &nodeId : result.sparseObservationNodeIds)
        RFileIO::readBinary(inFile,nodeId);

    result.validate();
    setup = std::move(result);
}

void RFileIO::writeAscii(RSaveFile &outFile,
                         const RStructuralThermalControlSetup &setup,
                         bool addNewLine)
{
    setup.validate();
    const bool writeThermalControlTail = outFile.getVersion() > RVersion(1,3,15);
    if (!writeThermalControlTail)
    {
        if (setup.enabled)
            throw RError(RError::Type::WriteFile,R_ERROR_REF,
                         "Thermal control data cannot be written to a pre-1.3.16 model file.");
        return;
    }

    struct PrecisionGuard
    {
        QTextStream &stream;
        int previous;
        ~PrecisionGuard() { stream.setRealNumberPrecision(previous); }
    } precisionGuard{outFile.getTextStream(),outFile.getTextStream().realNumberPrecision()};
    outFile.getTextStream().setRealNumberPrecision(17);
    bool first = true;
    auto write = [&](const auto &value)
    {
        if (!first && !addNewLine) RFileIO::writeAscii(outFile,' ',false);
        RFileIO::writeAscii(outFile,value,addNewLine);
        first = false;
    };

    write(setup.enabled);
    write(setup.thermalSubstepSeconds);
    write(setup.thermalSubstepsPerMechanicalMacrostep);
    write(setup.mechanicalMacrostepsPerControllerSample);
    write(uint(setup.channels.size()));
    for (const auto &channel : setup.channels)
    {
        write(channel.actuatorId);
        write(channel.heaterNodeId);
        write(channel.temperatureSensorId);
        write(channel.temperatureSensorNodeId);
        write(channel.turnOnBelowKelvin);
        write(channel.turnOffAtOrAboveKelvin);
        write(channel.onPowerWatts);
        write(channel.minimumPowerWatts);
        write(channel.maximumPowerWatts);
        write(channel.maximumPowerSlewWattsPerSecond);
        write(channel.maximumPowerSlewUnlimited);
    }
    write(uint(setup.sparseObservationNodeIds.size()));
    for (const QString &nodeId : setup.sparseObservationNodeIds)
        write(nodeId);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RStructuralThermalControlSetup &setup)
{
    setup.validate();
    const bool writeThermalControlTail = outFile.getVersion() > RVersion(1,3,15);
    if (!writeThermalControlTail)
    {
        if (setup.enabled)
            throw RError(RError::Type::WriteFile,R_ERROR_REF,
                         "Thermal control data cannot be written to a pre-1.3.16 model file.");
        return;
    }

    RFileIO::writeBinary(outFile,setup.enabled);
    RFileIO::writeBinary(outFile,setup.thermalSubstepSeconds);
    RFileIO::writeBinary(outFile,setup.thermalSubstepsPerMechanicalMacrostep);
    RFileIO::writeBinary(outFile,setup.mechanicalMacrostepsPerControllerSample);
    RFileIO::writeBinary(outFile,uint(setup.channels.size()));
    for (const auto &channel : setup.channels)
    {
        RFileIO::writeBinary(outFile,channel.actuatorId);
        RFileIO::writeBinary(outFile,channel.heaterNodeId);
        RFileIO::writeBinary(outFile,channel.temperatureSensorId);
        RFileIO::writeBinary(outFile,channel.temperatureSensorNodeId);
        RFileIO::writeBinary(outFile,channel.turnOnBelowKelvin);
        RFileIO::writeBinary(outFile,channel.turnOffAtOrAboveKelvin);
        RFileIO::writeBinary(outFile,channel.onPowerWatts);
        RFileIO::writeBinary(outFile,channel.minimumPowerWatts);
        RFileIO::writeBinary(outFile,channel.maximumPowerWatts);
        RFileIO::writeBinary(outFile,channel.maximumPowerSlewWattsPerSecond);
        RFileIO::writeBinary(outFile,channel.maximumPowerSlewUnlimited);
    }
    RFileIO::writeBinary(outFile,uint(setup.sparseObservationNodeIds.size()));
    for (const QString &nodeId : setup.sparseObservationNodeIds)
        RFileIO::writeBinary(outFile,nodeId);
}

void RFileIO::readAscii(RFile &inFile, RStructuralCableSetup &setup)
{
    RStructuralCableSetup result;
    uint memberCount = 0;
    RFileIO::readAscii(inFile,result.enabled);
    RFileIO::readAscii(inFile,result.meshFingerprint);
    RFileIO::readAscii(inFile,memberCount);
    if (memberCount > 1000000u)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,
                     "Excessive structural cable member count (%u).",memberCount);
    }
    result.members.resize(memberCount);
    for (RStructuralCableMember &member : result.members)
    {
        RFileIO::readAscii(inFile,member.elementId);
        RFileIO::readAscii(inFile,member.referenceLength);
        RFileIO::readAscii(inFile,member.initialTension);
    }
    RFileIO::readAscii(inFile,result.maxIterations);
    RFileIO::readAscii(inFile,result.maxLineSearchIterations);
    RFileIO::readAscii(inFile,result.absoluteForceTolerance);
    RFileIO::readAscii(inFile,result.relativeForceTolerance);
    result.validate();
    setup = std::move(result);
}

void RFileIO::readBinary(RFile &inFile, RStructuralCableSetup &setup)
{
    RStructuralCableSetup result;
    uint memberCount = 0;
    auto readExact = [&](void *destination, qint64 byteCount, const char *field)
    {
        if (byteCount < 0 || inFile.read(static_cast<char*>(destination),byteCount) != byteCount)
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF,
                         "Truncated structural cable binary data while reading %s.",field);
        }
    };
    auto readString = [&]()
    {
        uint byteCount = 0;
        readExact(&byteCount,qint64(sizeof(byteCount)),"mesh fingerprint length");
        // A valid fingerprint is empty or 64 ASCII hex bytes. Bound the read
        // before allocating so a corrupt length cannot request arbitrary RAM.
        if (byteCount > 64u)
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF,
                         "Invalid structural cable mesh fingerprint length (%u).",byteCount);
        }
        QByteArray bytes = inFile.read(qint64(byteCount));
        if (bytes.size() != qsizetype(byteCount))
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF,
                         "Truncated structural cable binary data while reading mesh fingerprint.");
        }
        return QString::fromUtf8(bytes.constData(),bytes.size());
    };

    static_assert(sizeof(bool) == sizeof(unsigned char),
                  "Binary model format requires a one-byte bool representation.");
    unsigned char encodedEnabled = 0;
    readExact(&encodedEnabled,qint64(sizeof(encodedEnabled)),"enabled flag");
    if (encodedEnabled > 1u)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,
                     "Invalid structural cable enabled flag (%u).",unsigned(encodedEnabled));
    }
    result.enabled = encodedEnabled != 0;
    result.meshFingerprint = readString();
    readExact(&memberCount,qint64(sizeof(memberCount)),"member count");
    if (memberCount > 1000000u)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,
                     "Excessive structural cable member count (%u).",memberCount);
    }
    result.members.resize(memberCount);
    for (RStructuralCableMember &member : result.members)
    {
        readExact(&member.elementId,qint64(sizeof(member.elementId)),"member element ID");
        readExact(&member.referenceLength,qint64(sizeof(member.referenceLength)),"member reference length");
        readExact(&member.initialTension,qint64(sizeof(member.initialTension)),"member initial tension");
    }
    readExact(&result.maxIterations,qint64(sizeof(result.maxIterations)),"maximum iterations");
    readExact(&result.maxLineSearchIterations,qint64(sizeof(result.maxLineSearchIterations)),"maximum line-search iterations");
    readExact(&result.absoluteForceTolerance,qint64(sizeof(result.absoluteForceTolerance)),"absolute force tolerance");
    readExact(&result.relativeForceTolerance,qint64(sizeof(result.relativeForceTolerance)),"relative force tolerance");
    result.validate();
    setup = std::move(result);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RStructuralCableSetup &setup, bool addNewLine)
{
    setup.validate();
    struct PrecisionGuard
    {
        QTextStream &stream;
        int previous;
        ~PrecisionGuard() { stream.setRealNumberPrecision(previous); }
    } precisionGuard{outFile.getTextStream(),outFile.getTextStream().realNumberPrecision()};
    outFile.getTextStream().setRealNumberPrecision(17);

    bool first = true;
    auto write = [&](const auto &value)
    {
        if (!first && !addNewLine) RFileIO::writeAscii(outFile,' ',false);
        RFileIO::writeAscii(outFile,value,addNewLine);
        first = false;
    };
    write(setup.enabled);
    write(setup.meshFingerprint);
    write(uint(setup.members.size()));
    for (const RStructuralCableMember &member : setup.members)
    {
        write(member.elementId);
        write(member.referenceLength);
        write(member.initialTension);
    }
    write(setup.maxIterations);
    write(setup.maxLineSearchIterations);
    write(setup.absoluteForceTolerance);
    write(setup.relativeForceTolerance);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RStructuralCableSetup &setup)
{
    setup.validate();
    RFileIO::writeBinary(outFile,setup.enabled);
    RFileIO::writeBinary(outFile,setup.meshFingerprint);
    RFileIO::writeBinary(outFile,uint(setup.members.size()));
    for (const RStructuralCableMember &member : setup.members)
    {
        RFileIO::writeBinary(outFile,member.elementId);
        RFileIO::writeBinary(outFile,member.referenceLength);
        RFileIO::writeBinary(outFile,member.initialTension);
    }
    RFileIO::writeBinary(outFile,setup.maxIterations);
    RFileIO::writeBinary(outFile,setup.maxLineSearchIterations);
    RFileIO::writeBinary(outFile,setup.absoluteForceTolerance);
    RFileIO::writeBinary(outFile,setup.relativeForceTolerance);
}

void RFileIO::readAscii(RFile &inFile, RStructuralPiezoSetup &setup)
{
    RStructuralPiezoSetup result;
    uint portCount = 0;
    QTextStream &stream = inFile.getTextStream();
    stream.skipWhiteSpace();
    QChar encodedEnabled;
    stream >> encodedEnabled;
    if (stream.status() != QTextStream::Ok)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,
                     "Failed to read structural piezo enabled flag.");
    }
    if (encodedEnabled.unicode() == 0 || encodedEnabled == QLatin1Char('0'))
    {
        result.enabled = false;
    }
    else if (encodedEnabled.unicode() == 1 || encodedEnabled == QLatin1Char('1'))
    {
        result.enabled = true;
    }
    else
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,
                     "Invalid structural piezo enabled flag.");
    }
    RFileIO::readAscii(inFile,result.meshFingerprint);
    RFileIO::readAscii(inFile,portCount);
    if (portCount > 1000000u)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,
                     "Excessive structural piezo port count (%u).",portCount);
    }
    result.ports.resize(portCount);
    auto readFiniteDouble = [&](const char *field)
    {
        QString token;
        stream >> token;
        if (stream.status() != QTextStream::Ok)
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF,
                         "Failed to read structural piezo %s.",field);
        }
        bool ok = false;
        const double value = token.toDouble(&ok);
        if (!ok || !std::isfinite(value))
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF,
                         "Invalid structural piezo %s.",field);
        }
        return value;
    };
    auto readPortId = [&]()
    {
        stream.skipWhiteSpace();
        if (stream.read(1) != QStringLiteral("\""))
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF,
                         "Invalid structural piezo port ID encoding.");
        }

        QString id;
        bool closed = false;
        while (!stream.atEnd())
        {
            const QString character = stream.read(1);
            if (character.isEmpty()) break;
            const QChar value = character.at(0);
            if (value == QLatin1Char('"'))
            {
                closed = true;
                break;
            }
            if (value == QLatin1Char('\n') || value == QLatin1Char('\r') || value == QChar(0))
            {
                throw RError(RError::Type::ReadFile,R_ERROR_REF,
                             "Invalid structural piezo port ID encoding.");
            }
            id.append(value);
            if (id.toUtf8().size() > 4096)
            {
                throw RError(RError::Type::ReadFile,R_ERROR_REF,
                             "Structural piezo port ID exceeds the maximum length.");
            }
        }
        if (!closed)
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF,
                         "Unterminated structural piezo port ID.");
        }
        return id;
    };
    for (RStructuralPiezoPort &port : result.ports)
    {
        port.id = readPortId();
        RFileIO::readAscii(inFile,port.elementId);
        port.coupling = readFiniteDouble("coupling");
        port.permittivity = readFiniteDouble("permittivity");
        port.voltage = readFiniteDouble("voltage");
    }
    if (inFile.getVersion() > RVersion(1,3,14))
    {
        int mode = 0;
        RFileIO::readAscii(inFile,mode);
        result.electricalMode = static_cast<RStructuralPiezoElectricalMode>(mode);
        uint boundaryCount = 0;
        RFileIO::readAscii(inFile,boundaryCount);
        if (boundaryCount > 1000000u)
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF,
                         "Excessive structural piezo nodal boundary count (%u).",boundaryCount);
        }
        result.nodalBoundaryConditions.resize(boundaryCount);
        for (RStructuralPiezoNodeBoundary &boundary : result.nodalBoundaryConditions)
        {
            int type = 0;
            RFileIO::readAscii(inFile,boundary.nodeId);
            RFileIO::readAscii(inFile,type);
            boundary.type = static_cast<RStructuralPiezoBoundaryType>(type);
            boundary.value = readFiniteDouble("nodal boundary value");
        }
    }
    if (inFile.getVersion() > RVersion(1,3,18))
    {
        auto readPassiveUInt = [&]()
        {
            QString token;
            stream >> token;
            bool ok = false;
            const uint value = token.toUInt(&ok);
            if (stream.status() != QTextStream::Ok || !ok || token.startsWith(QLatin1Char('-')))
            {
                throw RError(RError::Type::ReadFile,R_ERROR_REF,
                             "Invalid or truncated structural piezo passive element data.");
            }
            return value;
        };
        const uint passiveCount = readPassiveUInt();
        if (passiveCount > 1000000u)
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF,
                         "Excessive structural piezo passive element count (%u).",passiveCount);
        }
        result.passiveElementIds.resize(passiveCount);
        for (uint &elementId : result.passiveElementIds) elementId = readPassiveUInt();
    }
    result.validate();
    setup = std::move(result);
}

void RFileIO::readBinary(RFile &inFile, RStructuralPiezoSetup &setup)
{
    RStructuralPiezoSetup result;
    uint portCount = 0;
    auto readExact = [&](void *destination, qint64 byteCount, const char *field)
    {
        if (byteCount < 0 || inFile.read(static_cast<char*>(destination),byteCount) != byteCount)
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF,
                         "Truncated structural piezo binary data while reading %s.",field);
        }
    };
    auto readString = [&](uint maximumBytes, const char *field)
    {
        uint byteCount = 0;
        readExact(&byteCount,qint64(sizeof(byteCount)),field);
        if (byteCount > maximumBytes)
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF,
                         "Invalid structural piezo string length (%u) for %s.",byteCount,field);
        }
        const QByteArray bytes = inFile.read(qint64(byteCount));
        if (bytes.size() != qsizetype(byteCount))
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF,
                         "Truncated structural piezo binary data while reading %s.",field);
        }
        const QString value = QString::fromUtf8(bytes.constData(),bytes.size());
        if (value.toUtf8() != bytes)
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF,
                         "Invalid UTF-8 structural piezo string in %s.",field);
        }
        return value;
    };

    static_assert(sizeof(bool) == sizeof(unsigned char),
                  "Binary model format requires a one-byte bool representation.");
    unsigned char encodedEnabled = 0;
    readExact(&encodedEnabled,qint64(sizeof(encodedEnabled)),"enabled flag");
    if (encodedEnabled > 1u)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,
                     "Invalid structural piezo enabled flag (%u).",unsigned(encodedEnabled));
    }
    result.enabled = encodedEnabled != 0;
    result.meshFingerprint = readString(64u,"mesh fingerprint");
    readExact(&portCount,qint64(sizeof(portCount)),"port count");
    if (portCount > 1000000u)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,
                     "Excessive structural piezo port count (%u).",portCount);
    }
    result.ports.resize(portCount);
    for (RStructuralPiezoPort &port : result.ports)
    {
        port.id = readString(4096u,"port ID");
        readExact(&port.elementId,qint64(sizeof(port.elementId)),"port element ID");
        readExact(&port.coupling,qint64(sizeof(port.coupling)),"piezo coupling");
        readExact(&port.permittivity,qint64(sizeof(port.permittivity)),"piezo permittivity");
        readExact(&port.voltage,qint64(sizeof(port.voltage)),"port voltage");
    }
    if (inFile.getVersion() > RVersion(1,3,14))
    {
        int mode = 0;
        readExact(&mode,qint64(sizeof(mode)),"electrical mode");
        result.electricalMode = static_cast<RStructuralPiezoElectricalMode>(mode);
        uint boundaryCount = 0;
        readExact(&boundaryCount,qint64(sizeof(boundaryCount)),"nodal boundary count");
        if (boundaryCount > 1000000u)
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF,
                         "Excessive structural piezo nodal boundary count (%u).",boundaryCount);
        }
        result.nodalBoundaryConditions.resize(boundaryCount);
        for (RStructuralPiezoNodeBoundary &boundary : result.nodalBoundaryConditions)
        {
            int type = 0;
            readExact(&boundary.nodeId,qint64(sizeof(boundary.nodeId)),"nodal boundary node ID");
            readExact(&type,qint64(sizeof(type)),"nodal boundary type");
            boundary.type = static_cast<RStructuralPiezoBoundaryType>(type);
            readExact(&boundary.value,qint64(sizeof(boundary.value)),"nodal boundary value");
        }
    }
    if (inFile.getVersion() > RVersion(1,3,18))
    {
        uint passiveCount = 0;
        readExact(&passiveCount,qint64(sizeof(passiveCount)),"passive element count");
        if (passiveCount > 1000000u)
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF,
                         "Excessive structural piezo passive element count (%u).",passiveCount);
        }
        result.passiveElementIds.resize(passiveCount);
        for (uint &elementId : result.passiveElementIds)
        {
            readExact(&elementId,qint64(sizeof(elementId)),"passive element ID");
        }
    }
    result.validate();
    setup = std::move(result);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RStructuralPiezoSetup &setup, bool addNewLine)
{
    setup.validate();
    const bool writePassiveTail = outFile.getVersion() > RVersion(1,3,18);
    if (!writePassiveTail && !setup.passiveElementIds.empty())
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,
                     "Passive piezo element data cannot be written to a pre-1.3.19 model file.");
    }
    const bool writeNodalTail = outFile.getVersion() > RVersion(1,3,14);
    if (!writeNodalTail &&
        (setup.electricalMode != RStructuralPiezoElectricalMode::LegacyTerminalVoltage ||
         !setup.nodalBoundaryConditions.empty()))
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,
                     "Nodal-potential piezo data cannot be written to a pre-1.3.15 model file.");
    }
    struct PrecisionGuard
    {
        QTextStream &stream;
        int previous;
        ~PrecisionGuard() { stream.setRealNumberPrecision(previous); }
    } precisionGuard{outFile.getTextStream(),outFile.getTextStream().realNumberPrecision()};
    outFile.getTextStream().setRealNumberPrecision(17);

    bool first = true;
    auto write = [&](const auto &value)
    {
        if (!first && !addNewLine) RFileIO::writeAscii(outFile,' ',false);
        RFileIO::writeAscii(outFile,value,addNewLine);
        first = false;
    };
    write(setup.enabled);
    write(setup.meshFingerprint);
    write(uint(setup.ports.size()));
    for (const RStructuralPiezoPort &port : setup.ports)
    {
        write(QStringLiteral("\"") + port.id + QStringLiteral("\""));
        write(port.elementId);
        write(port.coupling);
        write(port.permittivity);
        write(port.voltage);
    }
    if (writeNodalTail)
    {
        write(int(setup.electricalMode));
        write(uint(setup.nodalBoundaryConditions.size()));
        for (const RStructuralPiezoNodeBoundary &boundary : setup.nodalBoundaryConditions)
        {
            write(boundary.nodeId);
            write(int(boundary.type));
            write(boundary.value);
        }
    }
    if (writePassiveTail)
    {
        write(uint(setup.passiveElementIds.size()));
        for (const uint elementId : setup.passiveElementIds) write(elementId);
    }
}

void RFileIO::writeBinary(RSaveFile &outFile, const RStructuralPiezoSetup &setup)
{
    setup.validate();
    const bool writePassiveTail = outFile.getVersion() > RVersion(1,3,18);
    if (!writePassiveTail && !setup.passiveElementIds.empty())
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,
                     "Passive piezo element data cannot be written to a pre-1.3.19 model file.");
    }
    const bool writeNodalTail = outFile.getVersion() > RVersion(1,3,14);
    if (!writeNodalTail &&
        (setup.electricalMode != RStructuralPiezoElectricalMode::LegacyTerminalVoltage ||
         !setup.nodalBoundaryConditions.empty()))
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,
                     "Nodal-potential piezo data cannot be written to a pre-1.3.15 model file.");
    }
    RFileIO::writeBinary(outFile,setup.enabled);
    RFileIO::writeBinary(outFile,setup.meshFingerprint);
    RFileIO::writeBinary(outFile,uint(setup.ports.size()));
    for (const RStructuralPiezoPort &port : setup.ports)
    {
        RFileIO::writeBinary(outFile,port.id);
        RFileIO::writeBinary(outFile,port.elementId);
        RFileIO::writeBinary(outFile,port.coupling);
        RFileIO::writeBinary(outFile,port.permittivity);
        RFileIO::writeBinary(outFile,port.voltage);
    }
    if (writeNodalTail)
    {
        RFileIO::writeBinary(outFile,int(setup.electricalMode));
        RFileIO::writeBinary(outFile,uint(setup.nodalBoundaryConditions.size()));
        for (const RStructuralPiezoNodeBoundary &boundary : setup.nodalBoundaryConditions)
        {
            RFileIO::writeBinary(outFile,boundary.nodeId);
            RFileIO::writeBinary(outFile,int(boundary.type));
            RFileIO::writeBinary(outFile,boundary.value);
        }
    }
    if (writePassiveTail)
    {
        RFileIO::writeBinary(outFile,uint(setup.passiveElementIds.size()));
        for (const uint elementId : setup.passiveElementIds) RFileIO::writeBinary(outFile,elementId);
    }
}

void RFileIO::readAscii(RFile &inFile, RStructuralDynamicsSetup &setup)
{
    bool enabled = false;
    int method = 0;
    double timeStep, rayleighAlpha, rayleighBeta, rhoInfinity, gamma, beta;
    uint nSteps, outputFrequency, nRatios;
    RFileIO::readAscii(inFile,enabled);
    RFileIO::readAscii(inFile,method);
    RFileIO::readAscii(inFile,timeStep);
    RFileIO::readAscii(inFile,nSteps);
    RFileIO::readAscii(inFile,outputFrequency);
    RFileIO::readAscii(inFile,rayleighAlpha);
    RFileIO::readAscii(inFile,rayleighBeta);
    RFileIO::readAscii(inFile,rhoInfinity);
    RFileIO::readAscii(inFile,gamma);
    RFileIO::readAscii(inFile,beta);
    RFileIO::readAscii(inFile,nRatios);
    if (nRatios > 100000)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,"Excessive modal damping ratio count (%u).",nRatios);
    }
    std::vector<double> ratios(nRatios);
    for (double &ratio : ratios) RFileIO::readAscii(inFile,ratio);
    RStructuralDynamicsSetup result;
    result.setEnabled(enabled);
    result.setMethod(static_cast<RStructuralTimeIntegrationMethod>(method));
    result.setTimeStep(timeStep);
    result.setNSteps(nSteps);
    result.setOutputFrequency(outputFrequency);
    result.setRayleighAlpha(rayleighAlpha);
    result.setRayleighBeta(rayleighBeta);
    result.setRhoInfinity(rhoInfinity);
    result.setNewmarkGamma(gamma);
    result.setNewmarkBeta(beta);
    result.setModalDampingRatios(ratios);
    result.validate();
    setup = result;
}

void RFileIO::readBinary(RFile &inFile, RStructuralDynamicsSetup &setup)
{
    bool enabled = false;
    int method = 0;
    double timeStep, rayleighAlpha, rayleighBeta, rhoInfinity, gamma, beta;
    uint nSteps, outputFrequency, nRatios;
    RFileIO::readBinary(inFile,enabled);
    RFileIO::readBinary(inFile,method);
    RFileIO::readBinary(inFile,timeStep);
    RFileIO::readBinary(inFile,nSteps);
    RFileIO::readBinary(inFile,outputFrequency);
    RFileIO::readBinary(inFile,rayleighAlpha);
    RFileIO::readBinary(inFile,rayleighBeta);
    RFileIO::readBinary(inFile,rhoInfinity);
    RFileIO::readBinary(inFile,gamma);
    RFileIO::readBinary(inFile,beta);
    RFileIO::readBinary(inFile,nRatios);
    if (nRatios > 100000)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,"Excessive modal damping ratio count (%u).",nRatios);
    }
    std::vector<double> ratios(nRatios);
    for (double &ratio : ratios) RFileIO::readBinary(inFile,ratio);
    RStructuralDynamicsSetup result;
    result.setEnabled(enabled);
    result.setMethod(static_cast<RStructuralTimeIntegrationMethod>(method));
    result.setTimeStep(timeStep);
    result.setNSteps(nSteps);
    result.setOutputFrequency(outputFrequency);
    result.setRayleighAlpha(rayleighAlpha);
    result.setRayleighBeta(rayleighBeta);
    result.setRhoInfinity(rhoInfinity);
    result.setNewmarkGamma(gamma);
    result.setNewmarkBeta(beta);
    result.setModalDampingRatios(ratios);
    result.validate();
    setup = result;
}

void RFileIO::writeAscii(RSaveFile &outFile, const RStructuralDynamicsSetup &setup, bool addNewLine)
{
    setup.validate();
    bool first = true;
    auto write = [&](const auto &value)
    {
        if (!first && !addNewLine) RFileIO::writeAscii(outFile,' ',false);
        RFileIO::writeAscii(outFile,value,addNewLine);
        first = false;
    };
    write(setup.getEnabled());
    write(int(setup.getMethod()));
    write(setup.getTimeStep());
    write(setup.getNSteps());
    write(setup.getOutputFrequency());
    write(setup.getRayleighAlpha());
    write(setup.getRayleighBeta());
    write(setup.getRhoInfinity());
    write(setup.getNewmarkGamma());
    write(setup.getNewmarkBeta());
    write(uint(setup.getModalDampingRatios().size()));
    // Modal ratios are replayed by index. Preserve their exact double values
    // without changing the legacy precision of the surrounding setup fields.
    struct PrecisionGuard
    {
        QTextStream &stream;
        int previous;
        ~PrecisionGuard() { stream.setRealNumberPrecision(previous); }
    } precisionGuard{outFile.getTextStream(),outFile.getTextStream().realNumberPrecision()};
    outFile.getTextStream().setRealNumberPrecision(17);
    for (double ratio : setup.getModalDampingRatios()) write(ratio);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RStructuralDynamicsSetup &setup)
{
    setup.validate();
    RFileIO::writeBinary(outFile,setup.getEnabled());
    RFileIO::writeBinary(outFile,int(setup.getMethod()));
    RFileIO::writeBinary(outFile,setup.getTimeStep());
    RFileIO::writeBinary(outFile,setup.getNSteps());
    RFileIO::writeBinary(outFile,setup.getOutputFrequency());
    RFileIO::writeBinary(outFile,setup.getRayleighAlpha());
    RFileIO::writeBinary(outFile,setup.getRayleighBeta());
    RFileIO::writeBinary(outFile,setup.getRhoInfinity());
    RFileIO::writeBinary(outFile,setup.getNewmarkGamma());
    RFileIO::writeBinary(outFile,setup.getNewmarkBeta());
    RFileIO::writeBinary(outFile,uint(setup.getModalDampingRatios().size()));
    for (double ratio : setup.getModalDampingRatios()) RFileIO::writeBinary(outFile,ratio);
}

void RFileIO::readAscii(RFile &inFile, RStructuralDynamicsRestartData &data)
{
    RStructuralDynamicsRestartData result;
    RFileIO::readAscii(inFile,result.available);
    if (result.available)
    {
        RFileIO::readAscii(inFile,result.completedSteps);
        RFileIO::readAscii(inFile,result.time);
        RFileIO::readAscii(inFile,result.fixedTimeStep);
        RFileIO::readAscii(inFile,result.meshNodeCount);
        RFileIO::readAscii(inFile,result.meshElementCount);
        RFileIO::readAscii(inFile,result.physicalDofCount);
        RFileIO::readAscii(inFile,result.meshFingerprint);
        RFileIO::readAscii(inFile,result.dofFingerprint);
        if (inFile.getVersion() > RVersion(1,3,7))
        {
            RFileIO::readAscii(inFile,result.systemFingerprint);
        }
        uint stateSize = 0;
        RFileIO::readAscii(inFile,stateSize);
        if (stateSize == 0 || stateSize > 20000000u || stateSize > result.physicalDofCount)
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF,"Invalid structural restart state size (%u).",stateSize);
        }
        result.q.resize(stateSize);
        result.v.resize(stateSize);
        result.a.resize(stateSize);
        result.externalForce.resize(stateSize);
        for (double &value : result.q) RFileIO::readAscii(inFile,value);
        for (double &value : result.v) RFileIO::readAscii(inFile,value);
        for (double &value : result.a) RFileIO::readAscii(inFile,value);
        for (double &value : result.externalForce) RFileIO::readAscii(inFile,value);
        RFileIO::readAscii(inFile,result.cumulativeExternalWork);
        RFileIO::readAscii(inFile,result.cumulativeDampingDissipation);
        RFileIO::readAscii(inFile,result.initialMechanicalEnergy);
        if (inFile.getVersion() > RVersion(1,3,9))
        {
            RFileIO::readAscii(inFile,result.supportResponseAvailable);
            if (result.supportResponseAvailable)
            {
                RFileIO::readAscii(inFile,result.supportFingerprint);
                uint supportCount = 0;
                RFileIO::readAscii(inFile,supportCount);
                if (supportCount > 3ull*result.meshNodeCount || supportCount > 20000000u)
                    throw RError(RError::Type::ReadFile,R_ERROR_REF,
                                 "Invalid structural support-response count (%u).",supportCount);
                result.supportReactions.resize(supportCount);
                result.supportPowers.resize(supportCount);
                result.cumulativeSupportWork.resize(supportCount);
                for (double &value : result.supportReactions) RFileIO::readAscii(inFile,value);
                for (double &value : result.supportPowers) RFileIO::readAscii(inFile,value);
                for (double &value : result.cumulativeSupportWork) RFileIO::readAscii(inFile,value);
            }
        }
        if (inFile.getVersion() > RVersion(1,3,10))
        {
            RFileIO::readAscii(inFile,result.controlAvailable);
            if (result.controlAvailable)
            {
                RFileIO::readAscii(inFile,result.controlProtocolVersion);
                RFileIO::readAscii(inFile,result.controlFingerprint);
                uint count = 0;
                RFileIO::readAscii(inFile,count);
                if (count == 0 || count > 4096u || count > 3ull*result.meshNodeCount)
                    throw RError(RError::Type::ReadFile,R_ERROR_REF,
                                 "Invalid structural control channel count (%u).",count);
                result.controlChannelIds.resize(count);
                result.controlLastCommands.resize(count);
                result.controlPowers.resize(count);
                result.cumulativeControlWork.resize(count);
                for (QString &value : result.controlChannelIds) RFileIO::readAscii(inFile,value);
                for (double &value : result.controlLastCommands) RFileIO::readAscii(inFile,value);
                for (double &value : result.controlPowers) RFileIO::readAscii(inFile,value);
                for (double &value : result.cumulativeControlWork) RFileIO::readAscii(inFile,value);
                QString encodedState, seed;
                RFileIO::readAscii(inFile,encodedState);
                RFileIO::readAscii(inFile,seed);
                if (encodedState.size() > 1398104)
                    throw RError(RError::Type::ReadFile,R_ERROR_REF,"Structural controller state exceeds 1 MiB.");
                result.controlControllerState = QByteArray::fromBase64(encodedState.toLatin1());
                bool ok = false;
                result.controlDeterministicSeed = seed.toULongLong(&ok,10);
                if (QString::fromLatin1(result.controlControllerState.toBase64()) != encodedState ||
                    !ok || seed != QString::number(result.controlDeterministicSeed))
                    throw RError(RError::Type::ReadFile,R_ERROR_REF,"Invalid structural control state or seed encoding.");
            }
        }
        // Version 1.3.7 did not bind reduced coordinates to the assembled
        // operators.  Consume its state for file compatibility, but never
        // allow that checkpoint to be used for a solver restart.
        if (inFile.getVersion() <= RVersion(1,3,7)) result.clear();
    }
    result.validate();
    data = std::move(result);
}

void RFileIO::readBinary(RFile &inFile, RStructuralDynamicsRestartData &data)
{
    RStructuralDynamicsRestartData result;
    RFileIO::readBinary(inFile,result.available);
    if (result.available)
    {
        RFileIO::readBinary(inFile,result.completedSteps);
        RFileIO::readBinary(inFile,result.time);
        RFileIO::readBinary(inFile,result.fixedTimeStep);
        RFileIO::readBinary(inFile,result.meshNodeCount);
        RFileIO::readBinary(inFile,result.meshElementCount);
        RFileIO::readBinary(inFile,result.physicalDofCount);
        RFileIO::readBinary(inFile,result.meshFingerprint);
        RFileIO::readBinary(inFile,result.dofFingerprint);
        if (inFile.getVersion() > RVersion(1,3,7))
        {
            RFileIO::readBinary(inFile,result.systemFingerprint);
        }
        uint stateSize = 0;
        RFileIO::readBinary(inFile,stateSize);
        if (stateSize == 0 || stateSize > 20000000u || stateSize > result.physicalDofCount)
        {
            throw RError(RError::Type::ReadFile,R_ERROR_REF,"Invalid structural restart state size (%u).",stateSize);
        }
        result.q.resize(stateSize);
        result.v.resize(stateSize);
        result.a.resize(stateSize);
        result.externalForce.resize(stateSize);
        for (double &value : result.q) RFileIO::readBinary(inFile,value);
        for (double &value : result.v) RFileIO::readBinary(inFile,value);
        for (double &value : result.a) RFileIO::readBinary(inFile,value);
        for (double &value : result.externalForce) RFileIO::readBinary(inFile,value);
        RFileIO::readBinary(inFile,result.cumulativeExternalWork);
        RFileIO::readBinary(inFile,result.cumulativeDampingDissipation);
        RFileIO::readBinary(inFile,result.initialMechanicalEnergy);
        if (inFile.getVersion() > RVersion(1,3,9))
        {
            RFileIO::readBinary(inFile,result.supportResponseAvailable);
            if (result.supportResponseAvailable)
            {
                RFileIO::readBinary(inFile,result.supportFingerprint);
                uint supportCount = 0;
                RFileIO::readBinary(inFile,supportCount);
                if (supportCount > 3ull*result.meshNodeCount || supportCount > 20000000u)
                    throw RError(RError::Type::ReadFile,R_ERROR_REF,
                                 "Invalid structural support-response count (%u).",supportCount);
                result.supportReactions.resize(supportCount);
                result.supportPowers.resize(supportCount);
                result.cumulativeSupportWork.resize(supportCount);
                for (double &value : result.supportReactions) RFileIO::readBinary(inFile,value);
                for (double &value : result.supportPowers) RFileIO::readBinary(inFile,value);
                for (double &value : result.cumulativeSupportWork) RFileIO::readBinary(inFile,value);
            }
        }
        if (inFile.getVersion() > RVersion(1,3,10))
        {
            RFileIO::readBinary(inFile,result.controlAvailable);
            if (result.controlAvailable)
            {
                RFileIO::readBinary(inFile,result.controlProtocolVersion);
                RFileIO::readBinary(inFile,result.controlFingerprint);
                uint count = 0;
                RFileIO::readBinary(inFile,count);
                if (count == 0 || count > 4096u || count > 3ull*result.meshNodeCount)
                    throw RError(RError::Type::ReadFile,R_ERROR_REF,
                                 "Invalid structural control channel count (%u).",count);
                result.controlChannelIds.resize(count);
                result.controlLastCommands.resize(count);
                result.controlPowers.resize(count);
                result.cumulativeControlWork.resize(count);
                for (QString &value : result.controlChannelIds) RFileIO::readBinary(inFile,value);
                for (double &value : result.controlLastCommands) RFileIO::readBinary(inFile,value);
                for (double &value : result.controlPowers) RFileIO::readBinary(inFile,value);
                for (double &value : result.cumulativeControlWork) RFileIO::readBinary(inFile,value);
                QString encodedState;
                RFileIO::readBinary(inFile,encodedState);
                if (encodedState.size() > 1398104)
                    throw RError(RError::Type::ReadFile,R_ERROR_REF,"Structural controller state exceeds 1 MiB.");
                result.controlControllerState = QByteArray::fromBase64(encodedState.toLatin1());
                if (QString::fromLatin1(result.controlControllerState.toBase64()) != encodedState)
                    throw RError(RError::Type::ReadFile,R_ERROR_REF,"Invalid structural controller state encoding.");
                uint low = 0, high = 0;
                RFileIO::readBinary(inFile,low);
                RFileIO::readBinary(inFile,high);
                result.controlDeterministicSeed = (quint64(high) << 32) | low;
            }
        }
        if (inFile.getVersion() <= RVersion(1,3,7)) result.clear();
    }
    result.validate();
    data = std::move(result);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RStructuralDynamicsRestartData &data, bool addNewLine)
{
    data.validate();
    struct PrecisionGuard
    {
        QTextStream &stream;
        int previous;
        ~PrecisionGuard() { stream.setRealNumberPrecision(previous); }
    } precisionGuard{outFile.getTextStream(),outFile.getTextStream().realNumberPrecision()};
    outFile.getTextStream().setRealNumberPrecision(17);
    bool first = true;
    auto write = [&](const auto &value)
    {
        if (!first && !addNewLine) RFileIO::writeAscii(outFile,' ',false);
        RFileIO::writeAscii(outFile,value,addNewLine);
        first = false;
    };
    write(data.available);
    if (!data.available) return;
    write(data.completedSteps);
    write(data.time);
    write(data.fixedTimeStep);
    write(data.meshNodeCount);
    write(data.meshElementCount);
    write(data.physicalDofCount);
    write(data.meshFingerprint);
    write(data.dofFingerprint);
    write(data.systemFingerprint);
    write(uint(data.q.size()));
    for (double value : data.q) write(value);
    for (double value : data.v) write(value);
    for (double value : data.a) write(value);
    for (double value : data.externalForce) write(value);
    write(data.cumulativeExternalWork);
    write(data.cumulativeDampingDissipation);
    write(data.initialMechanicalEnergy);
    write(data.supportResponseAvailable);
    if (data.supportResponseAvailable)
    {
        write(data.supportFingerprint);
        write(uint(data.supportReactions.size()));
        for (double value : data.supportReactions) write(value);
        for (double value : data.supportPowers) write(value);
        for (double value : data.cumulativeSupportWork) write(value);
    }
    write(data.controlAvailable);
    if (data.controlAvailable)
    {
        write(data.controlProtocolVersion);
        write(data.controlFingerprint);
        write(uint(data.controlChannelIds.size()));
        for (const QString &value : data.controlChannelIds) write(value);
        for (double value : data.controlLastCommands) write(value);
        for (double value : data.controlPowers) write(value);
        for (double value : data.cumulativeControlWork) write(value);
        write(QString::fromLatin1(data.controlControllerState.toBase64()));
        write(QString::number(data.controlDeterministicSeed));
    }
}

void RFileIO::writeBinary(RSaveFile &outFile, const RStructuralDynamicsRestartData &data)
{
    data.validate();
    RFileIO::writeBinary(outFile,data.available);
    if (!data.available) return;
    RFileIO::writeBinary(outFile,data.completedSteps);
    RFileIO::writeBinary(outFile,data.time);
    RFileIO::writeBinary(outFile,data.fixedTimeStep);
    RFileIO::writeBinary(outFile,data.meshNodeCount);
    RFileIO::writeBinary(outFile,data.meshElementCount);
    RFileIO::writeBinary(outFile,data.physicalDofCount);
    RFileIO::writeBinary(outFile,data.meshFingerprint);
    RFileIO::writeBinary(outFile,data.dofFingerprint);
    RFileIO::writeBinary(outFile,data.systemFingerprint);
    RFileIO::writeBinary(outFile,uint(data.q.size()));
    for (double value : data.q) RFileIO::writeBinary(outFile,value);
    for (double value : data.v) RFileIO::writeBinary(outFile,value);
    for (double value : data.a) RFileIO::writeBinary(outFile,value);
    for (double value : data.externalForce) RFileIO::writeBinary(outFile,value);
    RFileIO::writeBinary(outFile,data.cumulativeExternalWork);
    RFileIO::writeBinary(outFile,data.cumulativeDampingDissipation);
    RFileIO::writeBinary(outFile,data.initialMechanicalEnergy);
    RFileIO::writeBinary(outFile,data.supportResponseAvailable);
    if (data.supportResponseAvailable)
    {
        RFileIO::writeBinary(outFile,data.supportFingerprint);
        RFileIO::writeBinary(outFile,uint(data.supportReactions.size()));
        for (double value : data.supportReactions) RFileIO::writeBinary(outFile,value);
        for (double value : data.supportPowers) RFileIO::writeBinary(outFile,value);
        for (double value : data.cumulativeSupportWork) RFileIO::writeBinary(outFile,value);
    }
    RFileIO::writeBinary(outFile,data.controlAvailable);
    if (data.controlAvailable)
    {
        RFileIO::writeBinary(outFile,data.controlProtocolVersion);
        RFileIO::writeBinary(outFile,data.controlFingerprint);
        RFileIO::writeBinary(outFile,uint(data.controlChannelIds.size()));
        for (const QString &value : data.controlChannelIds) RFileIO::writeBinary(outFile,value);
        for (double value : data.controlLastCommands) RFileIO::writeBinary(outFile,value);
        for (double value : data.controlPowers) RFileIO::writeBinary(outFile,value);
        for (double value : data.cumulativeControlWork) RFileIO::writeBinary(outFile,value);
        RFileIO::writeBinary(outFile,QString::fromLatin1(data.controlControllerState.toBase64()));
        RFileIO::writeBinary(outFile,uint(data.controlDeterministicSeed & 0xffffffffu));
        RFileIO::writeBinary(outFile,uint(data.controlDeterministicSeed >> 32));
    }
}

void RFileIO::readAscii(RFile &inFile, RParallelExecutionConfig &config)
{
    int mode;
    uint threads;
    bool deterministic;
    RFileIO::readAscii(inFile,mode);
    RFileIO::readAscii(inFile,threads);
    RFileIO::readAscii(inFile,deterministic);
    RParallelExecutionConfig result;
    result.setMode(static_cast<RParallelExecutionMode>(mode));
    result.setRequestedThreads(threads);
    result.setDeterministicReduction(deterministic);
    result.validate();
    config = result;
}

void RFileIO::readBinary(RFile &inFile, RParallelExecutionConfig &config)
{
    int mode;
    uint threads;
    bool deterministic;
    RFileIO::readBinary(inFile,mode);
    RFileIO::readBinary(inFile,threads);
    RFileIO::readBinary(inFile,deterministic);
    RParallelExecutionConfig result;
    result.setMode(static_cast<RParallelExecutionMode>(mode));
    result.setRequestedThreads(threads);
    result.setDeterministicReduction(deterministic);
    result.validate();
    config = result;
}

void RFileIO::writeAscii(RSaveFile &outFile, const RParallelExecutionConfig &config, bool addNewLine)
{
    config.validate();
    RFileIO::writeAscii(outFile,int(config.getMode()),addNewLine);
    if (!addNewLine) RFileIO::writeAscii(outFile,' ',false);
    RFileIO::writeAscii(outFile,config.getRequestedThreads(),addNewLine);
    if (!addNewLine) RFileIO::writeAscii(outFile,' ',false);
    RFileIO::writeAscii(outFile,config.getDeterministicReduction(),addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RParallelExecutionConfig &config)
{
    config.validate();
    RFileIO::writeBinary(outFile,int(config.getMode()));
    RFileIO::writeBinary(outFile,config.getRequestedThreads());
    RFileIO::writeBinary(outFile,config.getDeterministicReduction());
}


/*********************************************************************
 *  RStructuralInteractionSetup                                      *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, RStructuralInteractionSetup &setup)
{
    auto readTarget = [&inFile](RStructuralTarget &target)
    {
        int value = 0;
        RFileIO::readAscii(inFile,value); target.scope = static_cast<RStructuralTargetScope>(value);
        RFileIO::readAscii(inFile,target.nodeId);
        RFileIO::readAscii(inFile,target.setName);
        RFileIO::readAscii(inFile,value); target.entityType = static_cast<REntityGroupType>(value);
    };
    uint count = 0;
    RFileIO::readAscii(inFile,count);
    setup.linearConstraints.resize(count);
    for (RLinearConstraint &constraint : setup.linearConstraints)
    {
        RFileIO::readAscii(inFile,constraint.id);
        RFileIO::readAscii(inFile,constraint.name);
        RFileIO::readAscii(inFile,constraint.rhs);
        uint nTerms = 0; RFileIO::readAscii(inFile,nTerms); constraint.terms.resize(nTerms);
        for (RLinearConstraintTerm &term : constraint.terms)
        {
            int dof = 0;
            RFileIO::readAscii(inFile,term.reference.nodeId);
            RFileIO::readAscii(inFile,dof); term.reference.dof = static_cast<RDofType>(dof);
            RFileIO::readAscii(inFile,term.coefficient);
        }
    }
    RFileIO::readAscii(inFile,count);
    setup.couplings.resize(count);
    for (RStructuralCoupling &coupling : setup.couplings)
    {
        int value = 0;
        RFileIO::readAscii(inFile,coupling.id); RFileIO::readAscii(inFile,coupling.name);
        RFileIO::readAscii(inFile,value); coupling.type = static_cast<RStructuralCouplingType>(value);
        RFileIO::readAscii(inFile,coupling.referenceNode);
        RFileIO::readAscii(inFile,coupling.masterSet); RFileIO::readAscii(inFile,coupling.slaveSet);
        RFileIO::readAscii(inFile,value); coupling.entityType = static_cast<REntityGroupType>(value);
        uint nDofs = 0; RFileIO::readAscii(inFile,nDofs); coupling.dofs.resize(nDofs);
        for (RDofType &dof : coupling.dofs) { RFileIO::readAscii(inFile,value); dof = static_cast<RDofType>(value); }
        RFileIO::readAscii(inFile,coupling.tolerance);
    }
    RFileIO::readAscii(inFile,count);
    setup.sensors.resize(count);
    for (RStructuralSensor &sensor : setup.sensors)
    {
        int value = 0;
        RFileIO::readAscii(inFile,sensor.id); RFileIO::readAscii(inFile,sensor.name); readTarget(sensor.target);
        RFileIO::readAscii(inFile,value); sensor.quantity = static_cast<RStructuralSensorQuantity>(value);
        RFileIO::readAscii(inFile,value); sensor.aggregation = static_cast<RStructuralSensorAggregation>(value);
        RFileIO::readAscii(inFile,value); sensor.frame = static_cast<RStructuralFrame>(value);
        RFileIO::readAscii(inFile,sensor.direction); RFileIO::readAscii(inFile,sensor.unit);
    }
    RFileIO::readAscii(inFile,count);
    setup.actuators.resize(count);
    for (RStructuralActuator &actuator : setup.actuators)
    {
        int value = 0;
        RFileIO::readAscii(inFile,actuator.id); RFileIO::readAscii(inFile,actuator.name);
        RFileIO::readAscii(inFile,value); actuator.type = static_cast<RStructuralActuatorType>(value);
        readTarget(actuator.target);
        RFileIO::readAscii(inFile,value); actuator.variable = static_cast<RVariableType>(value);
        RFileIO::readAscii(inFile,value); actuator.frame = static_cast<RStructuralFrame>(value);
        RFileIO::readAscii(inFile,actuator.direction); RFileIO::readAscii(inFile,actuator.unit);
        RFileIO::readAscii(inFile,actuator.minimum); RFileIO::readAscii(inFile,actuator.maximum); RFileIO::readAscii(inFile,actuator.rateLimit);
    }
    setup.prescribedMotions.clear();
    if (inFile.getVersion() > RVersion(1,3,8))
    {
        RFileIO::readAscii(inFile,count);
        setup.prescribedMotions.resize(count);
        std::set<std::pair<uint,int>> targets;
        std::set<QString> ids;
        for (RStructuralPrescribedMotion &motion : setup.prescribedMotions)
        {
            int dof = 0;
            uint nKnots = 0;
            RFileIO::readAscii(inFile,motion.id);
            RFileIO::readAscii(inFile,motion.target.nodeId);
            RFileIO::readAscii(inFile,dof);
            motion.target.dof = static_cast<RDofType>(dof);
            RFileIO::readAscii(inFile,nKnots);
            motion.knots.resize(nKnots);
            for (RStructuralMotionKnot &knot : motion.knots)
            {
                RFileIO::readAscii(inFile,knot.time);
                RFileIO::readAscii(inFile,knot.displacement);
                RFileIO::readAscii(inFile,knot.velocity);
                RFileIO::readAscii(inFile,knot.acceleration);
            }
            motion.validate();
            if (!ids.insert(motion.id).second ||
                !targets.insert({motion.target.nodeId,dof}).second)
            {
                throw RError(RError::Type::ReadFile,R_ERROR_REF,
                             "Duplicate prescribed motion ID or nodal DOF in model file.");
            }
        }
    }
}

void RFileIO::readBinary(RFile &inFile, RStructuralInteractionSetup &setup)
{
    auto readTarget = [&inFile](RStructuralTarget &target)
    {
        int value = 0;
        RFileIO::readBinary(inFile,value); target.scope = static_cast<RStructuralTargetScope>(value);
        RFileIO::readBinary(inFile,target.nodeId); RFileIO::readBinary(inFile,target.setName);
        RFileIO::readBinary(inFile,value); target.entityType = static_cast<REntityGroupType>(value);
    };
    uint count = 0;
    RFileIO::readBinary(inFile,count); setup.linearConstraints.resize(count);
    for (RLinearConstraint &constraint : setup.linearConstraints)
    {
        RFileIO::readBinary(inFile,constraint.id); RFileIO::readBinary(inFile,constraint.name); RFileIO::readBinary(inFile,constraint.rhs);
        uint nTerms = 0; RFileIO::readBinary(inFile,nTerms); constraint.terms.resize(nTerms);
        for (RLinearConstraintTerm &term : constraint.terms)
        {
            int dof = 0; RFileIO::readBinary(inFile,term.reference.nodeId); RFileIO::readBinary(inFile,dof);
            term.reference.dof = static_cast<RDofType>(dof); RFileIO::readBinary(inFile,term.coefficient);
        }
    }
    RFileIO::readBinary(inFile,count); setup.couplings.resize(count);
    for (RStructuralCoupling &coupling : setup.couplings)
    {
        int value = 0;
        RFileIO::readBinary(inFile,coupling.id); RFileIO::readBinary(inFile,coupling.name);
        RFileIO::readBinary(inFile,value); coupling.type = static_cast<RStructuralCouplingType>(value);
        RFileIO::readBinary(inFile,coupling.referenceNode); RFileIO::readBinary(inFile,coupling.masterSet); RFileIO::readBinary(inFile,coupling.slaveSet);
        RFileIO::readBinary(inFile,value); coupling.entityType = static_cast<REntityGroupType>(value);
        uint nDofs = 0; RFileIO::readBinary(inFile,nDofs); coupling.dofs.resize(nDofs);
        for (RDofType &dof : coupling.dofs) { RFileIO::readBinary(inFile,value); dof = static_cast<RDofType>(value); }
        RFileIO::readBinary(inFile,coupling.tolerance);
    }
    RFileIO::readBinary(inFile,count); setup.sensors.resize(count);
    for (RStructuralSensor &sensor : setup.sensors)
    {
        int value = 0;
        RFileIO::readBinary(inFile,sensor.id); RFileIO::readBinary(inFile,sensor.name); readTarget(sensor.target);
        RFileIO::readBinary(inFile,value); sensor.quantity = static_cast<RStructuralSensorQuantity>(value);
        RFileIO::readBinary(inFile,value); sensor.aggregation = static_cast<RStructuralSensorAggregation>(value);
        RFileIO::readBinary(inFile,value); sensor.frame = static_cast<RStructuralFrame>(value);
        RFileIO::readBinary(inFile,sensor.direction); RFileIO::readBinary(inFile,sensor.unit);
    }
    RFileIO::readBinary(inFile,count); setup.actuators.resize(count);
    for (RStructuralActuator &actuator : setup.actuators)
    {
        int value = 0;
        RFileIO::readBinary(inFile,actuator.id); RFileIO::readBinary(inFile,actuator.name);
        RFileIO::readBinary(inFile,value); actuator.type = static_cast<RStructuralActuatorType>(value); readTarget(actuator.target);
        RFileIO::readBinary(inFile,value); actuator.variable = static_cast<RVariableType>(value);
        RFileIO::readBinary(inFile,value); actuator.frame = static_cast<RStructuralFrame>(value);
        RFileIO::readBinary(inFile,actuator.direction); RFileIO::readBinary(inFile,actuator.unit);
        RFileIO::readBinary(inFile,actuator.minimum); RFileIO::readBinary(inFile,actuator.maximum); RFileIO::readBinary(inFile,actuator.rateLimit);
    }
    setup.prescribedMotions.clear();
    if (inFile.getVersion() > RVersion(1,3,8))
    {
        RFileIO::readBinary(inFile,count);
        setup.prescribedMotions.resize(count);
        std::set<std::pair<uint,int>> targets;
        std::set<QString> ids;
        for (RStructuralPrescribedMotion &motion : setup.prescribedMotions)
        {
            int dof = 0;
            uint nKnots = 0;
            RFileIO::readBinary(inFile,motion.id);
            RFileIO::readBinary(inFile,motion.target.nodeId);
            RFileIO::readBinary(inFile,dof);
            motion.target.dof = static_cast<RDofType>(dof);
            RFileIO::readBinary(inFile,nKnots);
            motion.knots.resize(nKnots);
            for (RStructuralMotionKnot &knot : motion.knots)
            {
                RFileIO::readBinary(inFile,knot.time);
                RFileIO::readBinary(inFile,knot.displacement);
                RFileIO::readBinary(inFile,knot.velocity);
                RFileIO::readBinary(inFile,knot.acceleration);
            }
            motion.validate();
            if (!ids.insert(motion.id).second ||
                !targets.insert({motion.target.nodeId,dof}).second)
            {
                throw RError(RError::Type::ReadFile,R_ERROR_REF,
                             "Duplicate prescribed motion ID or nodal DOF in model file.");
            }
        }
    }
}

void RFileIO::writeAscii(RSaveFile &outFile, const RStructuralInteractionSetup &setup, bool)
{
    auto writeString = [&outFile](const QString &value)
    {
        QString escaped(value);
        escaped.replace('"',QString());
        RFileIO::writeAscii(outFile,'"' + escaped + '"');
    };
    auto writeTarget = [&outFile](const RStructuralTarget &target)
    {
        RFileIO::writeAscii(outFile,int(target.scope)); RFileIO::writeAscii(outFile,target.nodeId);
        QString escaped(target.setName); escaped.replace('"',QString());
        RFileIO::writeAscii(outFile,'"' + escaped + '"'); RFileIO::writeAscii(outFile,int(target.entityType));
    };
    RFileIO::writeAscii(outFile,uint(setup.linearConstraints.size()));
    for (const RLinearConstraint &constraint : setup.linearConstraints)
    {
        writeString(constraint.id); writeString(constraint.name); RFileIO::writeAscii(outFile,constraint.rhs);
        RFileIO::writeAscii(outFile,uint(constraint.terms.size()));
        for (const RLinearConstraintTerm &term : constraint.terms)
        {
            RFileIO::writeAscii(outFile,term.reference.nodeId); RFileIO::writeAscii(outFile,int(term.reference.dof)); RFileIO::writeAscii(outFile,term.coefficient);
        }
    }
    RFileIO::writeAscii(outFile,uint(setup.couplings.size()));
    for (const RStructuralCoupling &coupling : setup.couplings)
    {
        writeString(coupling.id); writeString(coupling.name); RFileIO::writeAscii(outFile,int(coupling.type));
        RFileIO::writeAscii(outFile,coupling.referenceNode); writeString(coupling.masterSet); writeString(coupling.slaveSet);
        RFileIO::writeAscii(outFile,int(coupling.entityType)); RFileIO::writeAscii(outFile,uint(coupling.dofs.size()));
        for (RDofType dof : coupling.dofs) RFileIO::writeAscii(outFile,int(dof));
        RFileIO::writeAscii(outFile,coupling.tolerance);
    }
    RFileIO::writeAscii(outFile,uint(setup.sensors.size()));
    for (const RStructuralSensor &sensor : setup.sensors)
    {
        writeString(sensor.id); writeString(sensor.name); writeTarget(sensor.target);
        RFileIO::writeAscii(outFile,int(sensor.quantity)); RFileIO::writeAscii(outFile,int(sensor.aggregation)); RFileIO::writeAscii(outFile,int(sensor.frame));
        RFileIO::writeAscii(outFile,sensor.direction); writeString(sensor.unit);
    }
    RFileIO::writeAscii(outFile,uint(setup.actuators.size()));
    for (const RStructuralActuator &actuator : setup.actuators)
    {
        writeString(actuator.id); writeString(actuator.name); RFileIO::writeAscii(outFile,int(actuator.type)); writeTarget(actuator.target);
        RFileIO::writeAscii(outFile,int(actuator.variable)); RFileIO::writeAscii(outFile,int(actuator.frame)); RFileIO::writeAscii(outFile,actuator.direction);
        writeString(actuator.unit); RFileIO::writeAscii(outFile,actuator.minimum); RFileIO::writeAscii(outFile,actuator.maximum); RFileIO::writeAscii(outFile,actuator.rateLimit);
    }
    RFileIO::writeAscii(outFile,uint(setup.prescribedMotions.size()));
    // Motion knots define the interpolated trajectory, so preserve every double
    // across ASCII save/restart without changing legacy interaction fields.
    struct PrecisionGuard
    {
        QTextStream &stream;
        int previous;
        ~PrecisionGuard() { stream.setRealNumberPrecision(previous); }
    } precisionGuard{outFile.getTextStream(),outFile.getTextStream().realNumberPrecision()};
    outFile.getTextStream().setRealNumberPrecision(17);
    std::set<std::pair<uint,int>> motionTargets;
    std::set<QString> motionIds;
    for (const RStructuralPrescribedMotion &motion : setup.prescribedMotions)
    {
        motion.validate();
        if (!motionIds.insert(motion.id).second ||
            !motionTargets.insert({motion.target.nodeId,int(motion.target.dof)}).second)
        {
            throw RError(RError::Type::InvalidInput,R_ERROR_REF,
                         "Duplicate prescribed motion ID or nodal DOF.");
        }
        writeString(motion.id);
        RFileIO::writeAscii(outFile,motion.target.nodeId);
        RFileIO::writeAscii(outFile,int(motion.target.dof));
        RFileIO::writeAscii(outFile,uint(motion.knots.size()));
        for (const RStructuralMotionKnot &knot : motion.knots)
        {
            RFileIO::writeAscii(outFile,knot.time);
            RFileIO::writeAscii(outFile,knot.displacement);
            RFileIO::writeAscii(outFile,knot.velocity);
            RFileIO::writeAscii(outFile,knot.acceleration);
        }
    }
}

void RFileIO::writeBinary(RSaveFile &outFile, const RStructuralInteractionSetup &setup)
{
    auto writeTarget = [&outFile](const RStructuralTarget &target)
    {
        RFileIO::writeBinary(outFile,int(target.scope)); RFileIO::writeBinary(outFile,target.nodeId);
        RFileIO::writeBinary(outFile,target.setName); RFileIO::writeBinary(outFile,int(target.entityType));
    };
    RFileIO::writeBinary(outFile,uint(setup.linearConstraints.size()));
    for (const RLinearConstraint &constraint : setup.linearConstraints)
    {
        RFileIO::writeBinary(outFile,constraint.id); RFileIO::writeBinary(outFile,constraint.name); RFileIO::writeBinary(outFile,constraint.rhs);
        RFileIO::writeBinary(outFile,uint(constraint.terms.size()));
        for (const RLinearConstraintTerm &term : constraint.terms)
        {
            RFileIO::writeBinary(outFile,term.reference.nodeId); RFileIO::writeBinary(outFile,int(term.reference.dof)); RFileIO::writeBinary(outFile,term.coefficient);
        }
    }
    RFileIO::writeBinary(outFile,uint(setup.couplings.size()));
    for (const RStructuralCoupling &coupling : setup.couplings)
    {
        RFileIO::writeBinary(outFile,coupling.id); RFileIO::writeBinary(outFile,coupling.name); RFileIO::writeBinary(outFile,int(coupling.type));
        RFileIO::writeBinary(outFile,coupling.referenceNode); RFileIO::writeBinary(outFile,coupling.masterSet); RFileIO::writeBinary(outFile,coupling.slaveSet);
        RFileIO::writeBinary(outFile,int(coupling.entityType)); RFileIO::writeBinary(outFile,uint(coupling.dofs.size()));
        for (RDofType dof : coupling.dofs) RFileIO::writeBinary(outFile,int(dof));
        RFileIO::writeBinary(outFile,coupling.tolerance);
    }
    RFileIO::writeBinary(outFile,uint(setup.sensors.size()));
    for (const RStructuralSensor &sensor : setup.sensors)
    {
        RFileIO::writeBinary(outFile,sensor.id); RFileIO::writeBinary(outFile,sensor.name); writeTarget(sensor.target);
        RFileIO::writeBinary(outFile,int(sensor.quantity)); RFileIO::writeBinary(outFile,int(sensor.aggregation)); RFileIO::writeBinary(outFile,int(sensor.frame));
        RFileIO::writeBinary(outFile,sensor.direction); RFileIO::writeBinary(outFile,sensor.unit);
    }
    RFileIO::writeBinary(outFile,uint(setup.actuators.size()));
    for (const RStructuralActuator &actuator : setup.actuators)
    {
        RFileIO::writeBinary(outFile,actuator.id); RFileIO::writeBinary(outFile,actuator.name); RFileIO::writeBinary(outFile,int(actuator.type)); writeTarget(actuator.target);
        RFileIO::writeBinary(outFile,int(actuator.variable)); RFileIO::writeBinary(outFile,int(actuator.frame)); RFileIO::writeBinary(outFile,actuator.direction);
        RFileIO::writeBinary(outFile,actuator.unit); RFileIO::writeBinary(outFile,actuator.minimum); RFileIO::writeBinary(outFile,actuator.maximum); RFileIO::writeBinary(outFile,actuator.rateLimit);
    }
    RFileIO::writeBinary(outFile,uint(setup.prescribedMotions.size()));
    std::set<std::pair<uint,int>> motionTargets;
    std::set<QString> motionIds;
    for (const RStructuralPrescribedMotion &motion : setup.prescribedMotions)
    {
        motion.validate();
        if (!motionIds.insert(motion.id).second ||
            !motionTargets.insert({motion.target.nodeId,int(motion.target.dof)}).second)
        {
            throw RError(RError::Type::InvalidInput,R_ERROR_REF,
                         "Duplicate prescribed motion ID or nodal DOF.");
        }
        RFileIO::writeBinary(outFile,motion.id);
        RFileIO::writeBinary(outFile,motion.target.nodeId);
        RFileIO::writeBinary(outFile,int(motion.target.dof));
        RFileIO::writeBinary(outFile,uint(motion.knots.size()));
        for (const RStructuralMotionKnot &knot : motion.knots)
        {
            RFileIO::writeBinary(outFile,knot.time);
            RFileIO::writeBinary(outFile,knot.displacement);
            RFileIO::writeBinary(outFile,knot.velocity);
            RFileIO::writeBinary(outFile,knot.acceleration);
        }
    }
}


/*********************************************************************
 *  RRadiationResolution                                             *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, RRadiationResolution &radiationResolution)
{
    int iValue;
    inFile.getTextStream() >> iValue;
    if (inFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF, "Failed to read RRadiationResolution value.");
    }
    radiationResolution = RRadiationResolution(iValue);
}

void RFileIO::readBinary(RFile &inFile, RRadiationResolution &radiationResolution)
{
    inFile.read((char*)&radiationResolution,sizeof(RRadiationResolution));
    if (inFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::ReadFile,R_ERROR_REF,"Failed to read RRadiationResolution value.");
    }
}

void RFileIO::writeAscii(RSaveFile &outFile, const RRadiationResolution &radiationResolution, bool addNewLine)
{
    if (!addNewLine)
    {
        outFile.getTextStream() << int(radiationResolution);
    }
    else
    {
        outFile.getTextStream() << int(radiationResolution) << RConstants::endl;
    }
    if (outFile.getTextStream().status() != QTextStream::Ok)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write RRadiationResolution value.");
    }
}

void RFileIO::writeBinary(RSaveFile &outFile, const RRadiationResolution &radiationResolution)
{
    outFile.write((char*)&radiationResolution,sizeof(RRadiationResolution));
    if (outFile.error() != RFile::NoError)
    {
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write RRadiationResolution value.");
    }
}


/*********************************************************************
 *  RRadiationSetup                                                  *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, RRadiationSetup &radiationSetup)
{
    RFileIO::readAscii(inFile,radiationSetup.resolution);
    RFileIO::readAscii(inFile,radiationSetup.viewFactorMatrixFile);
}

void RFileIO::readBinary(RFile &inFile, RRadiationSetup &radiationSetup)
{
    RFileIO::readBinary(inFile,radiationSetup.resolution);
    RFileIO::readBinary(inFile,radiationSetup.viewFactorMatrixFile);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RRadiationSetup &radiationSetup, bool addNewLine)
{
    RFileIO::writeAscii(outFile,radiationSetup.resolution,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,radiationSetup.viewFactorMatrixFile,addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RRadiationSetup &radiationSetup)
{
    RFileIO::writeBinary(outFile,radiationSetup.resolution);
    RFileIO::writeBinary(outFile,radiationSetup.viewFactorMatrixFile);
}


/*********************************************************************
 *  RModalSetup                                                      *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, RModalSetup &modalSetup)
{
    RFileIO::readAscii(inFile,modalSetup.method);
    RFileIO::readAscii(inFile,modalSetup.nIterations);
    RFileIO::readAscii(inFile,modalSetup.nModesToExtract);
    RFileIO::readAscii(inFile,modalSetup.convergenceValue);
    RFileIO::readAscii(inFile,modalSetup.mode);
    RFileIO::readAscii(inFile,modalSetup.frequency);
}

void RFileIO::readBinary(RFile &inFile, RModalSetup &modalSetup)
{
    RFileIO::readBinary(inFile,modalSetup.method);
    RFileIO::readBinary(inFile,modalSetup.nIterations);
    RFileIO::readBinary(inFile,modalSetup.nModesToExtract);
    RFileIO::readBinary(inFile,modalSetup.convergenceValue);
    RFileIO::readBinary(inFile,modalSetup.mode);
    RFileIO::readBinary(inFile,modalSetup.frequency);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RModalSetup &modalSetup, bool addNewLine)
{
    RFileIO::writeAscii(outFile,modalSetup.method,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,modalSetup.nIterations,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,modalSetup.nModesToExtract,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,modalSetup.convergenceValue,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,modalSetup.mode,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,modalSetup.frequency,addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RModalSetup &modalSetup)
{
    RFileIO::writeBinary(outFile,modalSetup.method);
    RFileIO::writeBinary(outFile,modalSetup.nIterations);
    RFileIO::writeBinary(outFile,modalSetup.nModesToExtract);
    RFileIO::writeBinary(outFile,modalSetup.convergenceValue);
    RFileIO::writeBinary(outFile,modalSetup.mode);
    RFileIO::writeBinary(outFile,modalSetup.frequency);
}


/*********************************************************************
 *  RMeshSetup                                                       *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, RMeshSetup &meshSetup)
{
    meshSetup.variables.clear();

    uint nVariables = 0;
    RFileIO::readAscii(inFile,nVariables);
    for (uint i=0;i<nVariables;i++)
    {
        RVariableType variableType;
        RFileIO::readAscii(inFile,variableType);
        meshSetup.variables.insert(variableType);
    }
    RFileIO::readAscii(inFile,meshSetup.minEdgeLength);
    RFileIO::readAscii(inFile,meshSetup.maxEdgeLength);
}

void RFileIO::readBinary(RFile &inFile, RMeshSetup &meshSetup)
{
    meshSetup.variables.clear();

    uint nVariables = 0;
    RFileIO::readBinary(inFile,nVariables);
    for (uint i=0;i<nVariables;i++)
    {
        RVariableType variableType;
        RFileIO::readBinary(inFile,variableType);
        meshSetup.variables.insert(variableType);
    }
    RFileIO::readBinary(inFile,meshSetup.minEdgeLength);
    RFileIO::readBinary(inFile,meshSetup.maxEdgeLength);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RMeshSetup &meshSetup, bool addNewLine)
{
    struct PrecisionGuard
    {
        QTextStream &stream;
        int previous;
        ~PrecisionGuard() { stream.setRealNumberPrecision(previous); }
    } precisionGuard{outFile.getTextStream(),outFile.getTextStream().realNumberPrecision()};
    // GUI initialization retains nonzero bbox sizing seeds on reload. Default
    // text precision would overwrite their exact values and break strict
    // UNBOUND rebinding verification. Keep only this record round-trip safe.
    outFile.getTextStream().setRealNumberPrecision(17);

    RFileIO::writeAscii(outFile,uint(meshSetup.variables.size()),addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    foreach (RVariableType variableType, meshSetup.variables)
    {
        RFileIO::writeAscii(outFile,variableType,addNewLine);
        if (!addNewLine)
        {
            RFileIO::writeAscii(outFile,' ',false);
        }
    }
    RFileIO::writeAscii(outFile,meshSetup.minEdgeLength,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,meshSetup.maxEdgeLength,addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RMeshSetup &meshSetup)
{
    RFileIO::writeBinary(outFile,uint(meshSetup.variables.size()));
    foreach (RVariableType variableType, meshSetup.variables)
    {
        RFileIO::writeBinary(outFile,variableType);
    }
    RFileIO::writeBinary(outFile,meshSetup.minEdgeLength);
    RFileIO::writeBinary(outFile,meshSetup.maxEdgeLength);
}


/*********************************************************************
 *  RBook                                                            *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, RBook &book)
{
    RFileIO::readAscii(inFile,book.book,true);
}

void RFileIO::readBinary(RFile &inFile, RBook &book)
{
    RFileIO::readBinary(inFile,book.book,true);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RBook &book, bool addNewLine)
{
    RFileIO::writeAscii(outFile,book.book,true,addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RBook &book)
{
    RFileIO::writeBinary(outFile,book.book,true);
}


/*********************************************************************
 *  RPatch                                                           *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, RPatch &patch)
{
    RFileIO::readAscii(inFile,patch.surfaceID);
    RFileIO::readAscii(inFile,patch.elementIDs,true);
}

void RFileIO::readBinary(RFile &inFile, RPatch &patch)
{
    RFileIO::readBinary(inFile,patch.surfaceID);
    RFileIO::readBinary(inFile,patch.elementIDs,true);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RPatch &patch, bool addNewLine)
{
    RFileIO::writeAscii(outFile,patch.surfaceID,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,patch.elementIDs,true,addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RPatch &patch)
{
    RFileIO::writeBinary(outFile,patch.surfaceID);
    RFileIO::writeBinary(outFile,patch.elementIDs,true);
}


/*********************************************************************
 *  RPatchBook                                                       *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, RPatchBook &patchBook)
{
    RFileIO::readAscii(inFile,patchBook.patches);
    uint nElements = 0;
    RFileIO::readAscii(inFile,nElements);
    for (uint i=0;i<nElements;i++)
    {
        uint key = 0;
        uint value = 0;
        RFileIO::readAscii(inFile,key);
        RFileIO::readAscii(inFile,value);
        patchBook.elementPatchMap[key] = value;
    }
}

void RFileIO::readBinary(RFile &inFile, RPatchBook &patchBook)
{
    RFileIO::readBinary(inFile,patchBook.patches);
    uint nElements = 0;
    RFileIO::readBinary(inFile,nElements);
    for (uint i=0;i<nElements;i++)
    {
        uint key = 0;
        uint value = 0;
        RFileIO::readBinary(inFile,key);
        RFileIO::readBinary(inFile,value);
        patchBook.elementPatchMap[key] = value;
    }
}

void RFileIO::writeAscii(RSaveFile &outFile, const RPatchBook &patchBook, bool addNewLine)
{
    RFileIO::writeAscii(outFile,patchBook.patches,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    uint nElements = uint(patchBook.elementPatchMap.size());
    RFileIO::writeAscii(outFile,nElements,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    std::map<uint,uint>::const_iterator iter;
    uint i = 0;
    for (iter=patchBook.elementPatchMap.begin();iter!=patchBook.elementPatchMap.end();++iter)
    {
        if (i < nElements)
        {
            RFileIO::writeAscii(outFile,uint(iter->first),addNewLine);
            RFileIO::writeAscii(outFile,uint(iter->second),addNewLine);
            if (i < (nElements - 1) && !addNewLine)
            {
                RFileIO::writeAscii(outFile,' ',false);
            }
        }
        i++;
    }
}

void RFileIO::writeBinary(RSaveFile &outFile, const RPatchBook &patchBook)
{
    RFileIO::writeBinary(outFile,patchBook.patches);
    uint nElements = uint(patchBook.elementPatchMap.size());
    RFileIO::writeBinary(outFile,nElements);
    std::map<uint,uint>::const_iterator iter;
    uint i = 0;
    for (iter=patchBook.elementPatchMap.begin();iter!=patchBook.elementPatchMap.end();++iter)
    {
        if (i < nElements)
        {
            RFileIO::writeBinary(outFile,uint(iter->first));
            RFileIO::writeBinary(outFile,uint(iter->second));
        }
        i++;
    }
}


/*********************************************************************
 *  RPatchInput                                                      *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, RPatchInput &patchInput)
{
    RFileIO::readAscii(inFile,patchInput.patchArea);
    RFileIO::readAscii(inFile,patchInput.patchSize);
    RFileIO::readAscii(inFile,patchInput.separationAngle);
    RFileIO::readAscii(inFile,patchInput.emitter);
    RFileIO::readAscii(inFile,patchInput.receiver);
}

void RFileIO::readBinary(RFile &inFile, RPatchInput &patchInput)
{
    RFileIO::readBinary(inFile,patchInput.patchArea);
    RFileIO::readBinary(inFile,patchInput.patchSize);
    RFileIO::readBinary(inFile,patchInput.separationAngle);
    RFileIO::readBinary(inFile,patchInput.emitter);
    RFileIO::readBinary(inFile,patchInput.receiver);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RPatchInput &patchInput, bool addNewLine)
{
    RFileIO::writeAscii(outFile,patchInput.patchArea,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,patchInput.patchSize,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,patchInput.separationAngle,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,patchInput.emitter,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,patchInput.receiver,addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RPatchInput &patchInput)
{
    RFileIO::writeBinary(outFile,patchInput.patchArea);
    RFileIO::writeBinary(outFile,patchInput.patchSize);
    RFileIO::writeBinary(outFile,patchInput.separationAngle);
    RFileIO::writeBinary(outFile,patchInput.emitter);
    RFileIO::writeBinary(outFile,patchInput.receiver);
}


/*********************************************************************
 *  RViewFactorMatrixHeader                                          *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, RViewFactorMatrixHeader &viewFactorMatrixHeader)
{
    RFileIO::readAscii(inFile,viewFactorMatrixHeader.patchInput);
    RFileIO::readAscii(inFile,viewFactorMatrixHeader.hemicubeResolution);
    RFileIO::readAscii(inFile,viewFactorMatrixHeader.nElements);
}

void RFileIO::readBinary(RFile &inFile, RViewFactorMatrixHeader &viewFactorMatrixHeader)
{
    RFileIO::readBinary(inFile,viewFactorMatrixHeader.patchInput);
    RFileIO::readBinary(inFile,viewFactorMatrixHeader.hemicubeResolution);
    RFileIO::readBinary(inFile,viewFactorMatrixHeader.nElements);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RViewFactorMatrixHeader &viewFactorMatrixHeader, bool addNewLine)
{
    RFileIO::writeAscii(outFile,viewFactorMatrixHeader.patchInput,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,viewFactorMatrixHeader.hemicubeResolution,addNewLine);
    if (!addNewLine)
    {
        RFileIO::writeAscii(outFile,' ',false);
    }
    RFileIO::writeAscii(outFile,viewFactorMatrixHeader.nElements,addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RViewFactorMatrixHeader &viewFactorMatrixHeader)
{
    RFileIO::writeBinary(outFile,viewFactorMatrixHeader.patchInput);
    RFileIO::writeBinary(outFile,viewFactorMatrixHeader.hemicubeResolution);
    RFileIO::writeBinary(outFile,viewFactorMatrixHeader.nElements);
}


/*********************************************************************
 *  RViewFactorRow                                                   *
 *********************************************************************/

void RFileIO::readAscii(RFile &inFile, RViewFactorRow &viewFactorRow)
{
    RFileIO::readAscii(inFile,viewFactorRow.viewFactors);
}

void RFileIO::readBinary(RFile &inFile, RViewFactorRow &viewFactorRow)
{
    RFileIO::readBinary(inFile,viewFactorRow.viewFactors);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RViewFactorRow &viewFactorRow, bool addNewLine)
{
    RFileIO::writeAscii(outFile,viewFactorRow.viewFactors,addNewLine);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RViewFactorRow &viewFactorRow)
{
    RFileIO::writeBinary(outFile,viewFactorRow.viewFactors);
}

/*********************************************************************
 * Version 1.3.20 native mixed TRUSS1 persistence                     *
 *********************************************************************/

namespace
{
void trussReadError()
{
    throw RError(RError::Type::ReadFile,R_ERROR_REF,"Invalid or truncated native mixed TRUSS1 data.");
}

QString trussReadAsciiToken(RFile &file)
{
    QString token;
    file.getTextStream() >> token;
    if (file.getTextStream().status() != QTextStream::Ok) trussReadError();
    return token;
}

void trussReadAscii(RFile &file, uint &value)
{
    const QString token = trussReadAsciiToken(file);
    bool ok = false;
    value = token.toUInt(&ok,10);
    if (!ok || token.startsWith('-')) trussReadError();
}

void trussReadAscii(RFile &file, int &value)
{
    bool ok = false;
    value = trussReadAsciiToken(file).toInt(&ok,10);
    if (!ok) trussReadError();
}

void trussReadAscii(RFile &file, double &value)
{
    bool ok = false;
    value = trussReadAsciiToken(file).toDouble(&ok);
    if (!ok || !std::isfinite(value)) trussReadError();
}

void trussReadAscii(RFile &file, bool &value)
{
    QString token;
    file.getTextStream() >> token;
    if (file.getTextStream().status() != QTextStream::Ok) trussReadError();
    if (token != "0" && token != "1") trussReadError();
    value = token == "1";
}

void trussReadAscii(RFile &file, QString &value)
{
    QTextStream &stream = file.getTextStream();
    stream.skipWhiteSpace();
    QChar c;
    stream >> c;
    if (stream.status() != QTextStream::Ok || c != '"') trussReadError();
    value.clear();
    while (true)
    {
        stream >> c;
        if (stream.status() != QTextStream::Ok) trussReadError();
        if (c == '"') break;
        if (c == '\0' || c == '\n' || c == '\r' || value.size() >= 4096) trussReadError();
        value += c;
    }
}

void trussReadAscii(RFile &file, QByteArray &value)
{
    QString hex;
    trussReadAscii(file,hex);
    if (!hex.isEmpty() && hex.size() != 64) trussReadError();
    for (QChar c : hex)
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) trussReadError();
    value = QByteArray::fromHex(hex.toLatin1());
}

template<typename T> void trussReadBinary(RFile &file, T &value)
{
    if (file.read(reinterpret_cast<char*>(&value),sizeof(T)) != qint64(sizeof(T))
        || file.error() != RFile::NoError) trussReadError();
}

void trussReadBinary(RFile &file, double &value)
{
    if (file.read(reinterpret_cast<char*>(&value),sizeof(double)) != qint64(sizeof(double))
        || file.error() != RFile::NoError || !std::isfinite(value)) trussReadError();
}

void trussReadBinary(RFile &file, bool &value)
{
    unsigned char raw = 0;
    trussReadBinary(file,raw);
    if (raw > 1) trussReadError();
    value = raw == 1;
}

void trussReadBinary(RFile &file, QString &value)
{
    uint count = 0;
    trussReadBinary(file,count);
    if (count > 4096u || file.bytesAvailable() < qint64(count)) trussReadError();
    const QByteArray bytes = file.read(count);
    if (bytes.size() != qint64(count) || file.error() != RFile::NoError) trussReadError();
    value = QString::fromUtf8(bytes);
    if (value.toUtf8() != bytes || value.contains(QChar('\0'))
        || value.contains('"') || value.contains('\n') || value.contains('\r')) trussReadError();
}

void trussReadBinary(RFile &file, QByteArray &value)
{
    uint count = 0;
    trussReadBinary(file,count);
    if (count != 0 && count != 32) trussReadError();
    value = file.read(count);
    if (value.size() != qint64(count) || file.error() != RFile::NoError) trussReadError();
}

template<typename Read> uint trussReadCount(Read &read, uint limit = 1000000u)
{
    uint count = 0;
    read(count);
    if (count > limit) trussReadError();
    return count;
}

template<typename Read, typename T> void trussReadVector(Read &read, std::vector<T> &values,
                                                       uint limit = 1000000u)
{
    const uint count = trussReadCount(read,limit);
    values.resize(count);
    for (auto &value : values) read(value);
}

void trussWriteAsciiValue(RSaveFile &file, const QString &value, bool newLine)
{
    RFileIO::writeAscii(file,QString('"') + value + '"',newLine);
}

void trussWriteAsciiValue(RSaveFile &file, const QByteArray &value, bool newLine)
{
    trussWriteAsciiValue(file,QString::fromLatin1(value.toHex()),newLine);
}

void trussWriteAsciiValue(RSaveFile &file, bool value, bool newLine)
{
    RFileIO::writeAscii(file,uint(value ? 1 : 0),newLine);
}

template<typename T> void trussWriteAsciiValue(RSaveFile &file, const T &value, bool newLine)
{
    RFileIO::writeAscii(file,value,newLine);
}

void trussWriteBinaryValue(RSaveFile &file, const QByteArray &value)
{
    RFileIO::writeBinary(file,uint(value.size()));
    if (file.write(value) != value.size() || file.error() != RFile::NoError)
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write native mixed TRUSS1 system identity.");
}

template<typename T> void trussWriteBinaryValue(RSaveFile &file, const T &value)
{
    RFileIO::writeBinary(file,value);
}
}

void RFileIO::readAscii(RFile &inFile, RStructuralTrussDynamicsSetup &data)
{
    RStructuralTrussDynamicsSetup result;
    auto read = [&](auto &value) { trussReadAscii(inFile,value); };

    read(result.enabled);
    read(result.sourceFingerprint);
    result.passiveMembers.resize(trussReadCount(read));
    for (auto &member : result.passiveMembers)
    {
        read(member.physicalElementId);
        read(member.referenceLength);
    }
    result.activeMembers.resize(trussReadCount(read));
    for (auto &member : result.activeMembers)
    {
        read(member.physicalElementId);
        read(member.portId);
        read(member.referenceLength);
        read(member.axialStiffness);
        read(member.coupling);
        read(member.capacitance);
        int kind = 0;
        read(kind);
        if (kind < 0 || kind > 1) trussReadError();
        member.kind = static_cast<RStructuralTrussElectricalKind>(kind);
        read(member.initialValue);
        int massSource = 0;
        read(massSource);
        if (massSource < 0 || massSource > 1) trussReadError();
        member.massSource = static_cast<RStructuralTrussMassSource>(massSource);
        read(member.fixedMass);
    }
    result.cableMembers.resize(trussReadCount(read));
    for (auto &member : result.cableMembers)
    {
        read(member.physicalElementId);
        read(member.initialReferenceLength);
        read(member.initialTension);
        int massSource = 0;
        read(massSource);
        if (massSource < 0 || massSource > 1) trussReadError();
        member.massSource = static_cast<RStructuralTrussMassSource>(massSource);
        read(member.fixedMass);
    }
    result.heldCommands.resize(trussReadCount(read));
    uint targetCount = 0;
    auto targets = [&]()
    {
        const uint count = trussReadCount(read,1000000u - targetCount);
        targetCount += count;
        return count;
    };
    for (auto &command : result.heldCommands)
    {
        read(command.stepIndex);
        command.electrical.resize(targets());
        for (auto &target : command.electrical)
        {
            read(target.portId);
            read(target.value);
        }
        command.cables.resize(targets());
        for (auto &target : command.cables)
        {
            read(target.physicalElementId);
            read(target.referenceLength);
        }
        command.nodalForces.resize(targets());
        for (auto &target : command.nodalForces)
        {
            read(target.physicalNodeId);
            for (double &force : target.force) read(force);
        }
    }
    read(result.maxIterations);
    read(result.maxLineSearchIterations);
    read(result.absoluteForceTolerance);
    read(result.relativeForceTolerance);
    read(result.linearRelativeTolerance);
    result.validate();
    data = std::move(result);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RStructuralTrussDynamicsSetup &data, bool addNewLine)
{
    data.validate();
    if (outFile.getVersion() <= RVersion(1,3,19))
    {
        if (data.hasNonDefaultData())
            throw RError(RError::Type::WriteFile,R_ERROR_REF,"Native mixed TRUSS1 data requires model format 1.3.20.");
        return;
    }
    struct PrecisionGuard
    {
        QTextStream &stream;
        int previous;
        ~PrecisionGuard() { stream.setRealNumberPrecision(previous); }
    } precisionGuard{outFile.getTextStream(),outFile.getTextStream().realNumberPrecision()};
    outFile.getTextStream().setRealNumberPrecision(17);
    bool first = true;
    auto write = [&](const auto &value)
    {
        if (!first && !addNewLine) RFileIO::writeAscii(outFile,' ',false);
        trussWriteAsciiValue(outFile,value,addNewLine);
        first = false;
    };

    write(data.enabled);
    write(data.sourceFingerprint);
    write(uint(data.passiveMembers.size()));
    for (const auto &member : data.passiveMembers)
    {
        write(member.physicalElementId);
        write(member.referenceLength);
    }
    write(uint(data.activeMembers.size()));
    for (const auto &member : data.activeMembers)
    {
        write(member.physicalElementId);
        write(member.portId);
        write(member.referenceLength);
        write(member.axialStiffness);
        write(member.coupling);
        write(member.capacitance);
        write(int(member.kind));
        write(member.initialValue);
        write(int(member.massSource));
        write(member.fixedMass);
    }
    write(uint(data.cableMembers.size()));
    for (const auto &member : data.cableMembers)
    {
        write(member.physicalElementId);
        write(member.initialReferenceLength);
        write(member.initialTension);
        write(int(member.massSource));
        write(member.fixedMass);
    }
    write(uint(data.heldCommands.size()));
    for (const auto &command : data.heldCommands)
    {
        write(command.stepIndex);
        write(uint(command.electrical.size()));
        for (const auto &target : command.electrical)
        {
            write(target.portId);
            write(target.value);
        }
        write(uint(command.cables.size()));
        for (const auto &target : command.cables)
        {
            write(target.physicalElementId);
            write(target.referenceLength);
        }
        write(uint(command.nodalForces.size()));
        for (const auto &target : command.nodalForces)
        {
            write(target.physicalNodeId);
            for (double force : target.force) write(force);
        }
    }
    write(data.maxIterations);
    write(data.maxLineSearchIterations);
    write(data.absoluteForceTolerance);
    write(data.relativeForceTolerance);
    write(data.linearRelativeTolerance);
}

void RFileIO::readBinary(RFile &inFile, RStructuralTrussDynamicsSetup &data)
{
    RStructuralTrussDynamicsSetup result;
    auto read = [&](auto &value) { trussReadBinary(inFile,value); };

    read(result.enabled);
    read(result.sourceFingerprint);
    result.passiveMembers.resize(trussReadCount(read));
    for (auto &member : result.passiveMembers)
    {
        read(member.physicalElementId);
        read(member.referenceLength);
    }
    result.activeMembers.resize(trussReadCount(read));
    for (auto &member : result.activeMembers)
    {
        read(member.physicalElementId);
        read(member.portId);
        read(member.referenceLength);
        read(member.axialStiffness);
        read(member.coupling);
        read(member.capacitance);
        int kind = 0;
        read(kind);
        if (kind < 0 || kind > 1) trussReadError();
        member.kind = static_cast<RStructuralTrussElectricalKind>(kind);
        read(member.initialValue);
        int massSource = 0;
        read(massSource);
        if (massSource < 0 || massSource > 1) trussReadError();
        member.massSource = static_cast<RStructuralTrussMassSource>(massSource);
        read(member.fixedMass);
    }
    result.cableMembers.resize(trussReadCount(read));
    for (auto &member : result.cableMembers)
    {
        read(member.physicalElementId);
        read(member.initialReferenceLength);
        read(member.initialTension);
        int massSource = 0;
        read(massSource);
        if (massSource < 0 || massSource > 1) trussReadError();
        member.massSource = static_cast<RStructuralTrussMassSource>(massSource);
        read(member.fixedMass);
    }
    result.heldCommands.resize(trussReadCount(read));
    uint targetCount = 0;
    auto targets = [&]()
    {
        const uint count = trussReadCount(read,1000000u - targetCount);
        targetCount += count;
        return count;
    };
    for (auto &command : result.heldCommands)
    {
        read(command.stepIndex);
        command.electrical.resize(targets());
        for (auto &target : command.electrical)
        {
            read(target.portId);
            read(target.value);
        }
        command.cables.resize(targets());
        for (auto &target : command.cables)
        {
            read(target.physicalElementId);
            read(target.referenceLength);
        }
        command.nodalForces.resize(targets());
        for (auto &target : command.nodalForces)
        {
            read(target.physicalNodeId);
            for (double &force : target.force) read(force);
        }
    }
    read(result.maxIterations);
    read(result.maxLineSearchIterations);
    read(result.absoluteForceTolerance);
    read(result.relativeForceTolerance);
    read(result.linearRelativeTolerance);
    result.validate();
    data = std::move(result);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RStructuralTrussDynamicsSetup &data)
{
    data.validate();
    if (outFile.getVersion() <= RVersion(1,3,19))
    {
        if (data.hasNonDefaultData())
            throw RError(RError::Type::WriteFile,R_ERROR_REF,"Native mixed TRUSS1 data requires model format 1.3.20.");
        return;
    }
    auto write = [&](const auto &value) { trussWriteBinaryValue(outFile,value); };

    write(data.enabled);
    write(data.sourceFingerprint);
    write(uint(data.passiveMembers.size()));
    for (const auto &member : data.passiveMembers)
    {
        write(member.physicalElementId);
        write(member.referenceLength);
    }
    write(uint(data.activeMembers.size()));
    for (const auto &member : data.activeMembers)
    {
        write(member.physicalElementId);
        write(member.portId);
        write(member.referenceLength);
        write(member.axialStiffness);
        write(member.coupling);
        write(member.capacitance);
        write(int(member.kind));
        write(member.initialValue);
        write(int(member.massSource));
        write(member.fixedMass);
    }
    write(uint(data.cableMembers.size()));
    for (const auto &member : data.cableMembers)
    {
        write(member.physicalElementId);
        write(member.initialReferenceLength);
        write(member.initialTension);
        write(int(member.massSource));
        write(member.fixedMass);
    }
    write(uint(data.heldCommands.size()));
    for (const auto &command : data.heldCommands)
    {
        write(command.stepIndex);
        write(uint(command.electrical.size()));
        for (const auto &target : command.electrical)
        {
            write(target.portId);
            write(target.value);
        }
        write(uint(command.cables.size()));
        for (const auto &target : command.cables)
        {
            write(target.physicalElementId);
            write(target.referenceLength);
        }
        write(uint(command.nodalForces.size()));
        for (const auto &target : command.nodalForces)
        {
            write(target.physicalNodeId);
            for (double force : target.force) write(force);
        }
    }
    write(data.maxIterations);
    write(data.maxLineSearchIterations);
    write(data.absoluteForceTolerance);
    write(data.relativeForceTolerance);
    write(data.linearRelativeTolerance);
}

void RFileIO::readAscii(RFile &inFile, RStructuralTrussDynamicsRestartData &data)
{
    RStructuralTrussDynamicsRestartData result;
    auto read = [&](auto &value) { trussReadAscii(inFile,value); };
    read(result.available);
    read(result.completedSteps);
    read(result.runTargetSteps);
    read(result.initialTime);
    read(result.time);
    read(result.fixedTimeStep);
    read(result.nextCommandIndex);
    trussReadVector(read,result.orderedPhysicalNodeIds);
    trussReadVector(read,result.orderedMemberPhysicalIds);
    trussReadVector(read,result.orderedPortIds);
    read(result.sourceFingerprint);
    read(result.systemIdentity);
    trussReadVector(read,result.q,3000000u);
    trussReadVector(read,result.v,3000000u);
    trussReadVector(read,result.a,3000000u);
    trussReadVector(read,result.reactions,3000000u);
    trussReadVector(read,result.externalForce,3000000u);
    trussReadVector(read,result.memberInputKinds);
    trussReadVector(read,result.memberInputValues);
    trussReadVector(read,result.portVoltages);
    trussReadVector(read,result.portCharges);
    trussReadVector(read,result.portEnthalpies);
    read(result.storedEnergy);
    read(result.kineticEnergy);
    read(result.initialTotalEnergy);
    read(result.cumulativeExternalWork);
    read(result.cumulativeSupportWork);
    read(result.cumulativeElectricalWork);
    read(result.cumulativeRestLengthWork);
    result.validate();
    data = std::move(result);
}

void RFileIO::writeAscii(RSaveFile &outFile, const RStructuralTrussDynamicsRestartData &data, bool addNewLine)
{
    data.validate();
    if (outFile.getVersion() <= RVersion(1,3,19))
    {
        if (data.hasNonDefaultData())
            throw RError(RError::Type::WriteFile,R_ERROR_REF,"Native mixed TRUSS1 data requires model format 1.3.20.");
        return;
    }
    struct PrecisionGuard
    {
        QTextStream &stream;
        int previous;
        ~PrecisionGuard() { stream.setRealNumberPrecision(previous); }
    } precisionGuard{outFile.getTextStream(),outFile.getTextStream().realNumberPrecision()};
    outFile.getTextStream().setRealNumberPrecision(17);
    bool first = true;
    auto write = [&](const auto &value)
    {
        if (!first && !addNewLine) RFileIO::writeAscii(outFile,' ',false);
        trussWriteAsciiValue(outFile,value,addNewLine);
        first = false;
    };
    write(data.available);
    write(data.completedSteps);
    write(data.runTargetSteps);
    write(data.initialTime);
    write(data.time);
    write(data.fixedTimeStep);
    write(data.nextCommandIndex);
    write(uint(data.orderedPhysicalNodeIds.size()));
    for (const auto &value : data.orderedPhysicalNodeIds) write(value);
    write(uint(data.orderedMemberPhysicalIds.size()));
    for (const auto &value : data.orderedMemberPhysicalIds) write(value);
    write(uint(data.orderedPortIds.size()));
    for (const auto &value : data.orderedPortIds) write(value);
    write(data.sourceFingerprint);
    write(data.systemIdentity);
    write(uint(data.q.size()));
    for (const auto &value : data.q) write(value);
    write(uint(data.v.size()));
    for (const auto &value : data.v) write(value);
    write(uint(data.a.size()));
    for (const auto &value : data.a) write(value);
    write(uint(data.reactions.size()));
    for (const auto &value : data.reactions) write(value);
    write(uint(data.externalForce.size()));
    for (const auto &value : data.externalForce) write(value);
    write(uint(data.memberInputKinds.size()));
    for (const auto &value : data.memberInputKinds) write(value);
    write(uint(data.memberInputValues.size()));
    for (const auto &value : data.memberInputValues) write(value);
    write(uint(data.portVoltages.size()));
    for (const auto &value : data.portVoltages) write(value);
    write(uint(data.portCharges.size()));
    for (const auto &value : data.portCharges) write(value);
    write(uint(data.portEnthalpies.size()));
    for (const auto &value : data.portEnthalpies) write(value);
    write(data.storedEnergy);
    write(data.kineticEnergy);
    write(data.initialTotalEnergy);
    write(data.cumulativeExternalWork);
    write(data.cumulativeSupportWork);
    write(data.cumulativeElectricalWork);
    write(data.cumulativeRestLengthWork);
}

void RFileIO::readBinary(RFile &inFile, RStructuralTrussDynamicsRestartData &data)
{
    RStructuralTrussDynamicsRestartData result;
    auto read = [&](auto &value) { trussReadBinary(inFile,value); };
    read(result.available);
    read(result.completedSteps);
    read(result.runTargetSteps);
    read(result.initialTime);
    read(result.time);
    read(result.fixedTimeStep);
    read(result.nextCommandIndex);
    trussReadVector(read,result.orderedPhysicalNodeIds);
    trussReadVector(read,result.orderedMemberPhysicalIds);
    trussReadVector(read,result.orderedPortIds);
    read(result.sourceFingerprint);
    read(result.systemIdentity);
    trussReadVector(read,result.q,3000000u);
    trussReadVector(read,result.v,3000000u);
    trussReadVector(read,result.a,3000000u);
    trussReadVector(read,result.reactions,3000000u);
    trussReadVector(read,result.externalForce,3000000u);
    trussReadVector(read,result.memberInputKinds);
    trussReadVector(read,result.memberInputValues);
    trussReadVector(read,result.portVoltages);
    trussReadVector(read,result.portCharges);
    trussReadVector(read,result.portEnthalpies);
    read(result.storedEnergy);
    read(result.kineticEnergy);
    read(result.initialTotalEnergy);
    read(result.cumulativeExternalWork);
    read(result.cumulativeSupportWork);
    read(result.cumulativeElectricalWork);
    read(result.cumulativeRestLengthWork);
    result.validate();
    data = std::move(result);
}

void RFileIO::writeBinary(RSaveFile &outFile, const RStructuralTrussDynamicsRestartData &data)
{
    data.validate();
    if (outFile.getVersion() <= RVersion(1,3,19))
    {
        if (data.hasNonDefaultData())
            throw RError(RError::Type::WriteFile,R_ERROR_REF,"Native mixed TRUSS1 data requires model format 1.3.20.");
        return;
    }
    auto write = [&](const auto &value) { trussWriteBinaryValue(outFile,value); };
    write(data.available);
    write(data.completedSteps);
    write(data.runTargetSteps);
    write(data.initialTime);
    write(data.time);
    write(data.fixedTimeStep);
    write(data.nextCommandIndex);
    write(uint(data.orderedPhysicalNodeIds.size()));
    for (const auto &value : data.orderedPhysicalNodeIds) write(value);
    write(uint(data.orderedMemberPhysicalIds.size()));
    for (const auto &value : data.orderedMemberPhysicalIds) write(value);
    write(uint(data.orderedPortIds.size()));
    for (const auto &value : data.orderedPortIds) write(value);
    write(data.sourceFingerprint);
    write(data.systemIdentity);
    write(uint(data.q.size()));
    for (const auto &value : data.q) write(value);
    write(uint(data.v.size()));
    for (const auto &value : data.v) write(value);
    write(uint(data.a.size()));
    for (const auto &value : data.a) write(value);
    write(uint(data.reactions.size()));
    for (const auto &value : data.reactions) write(value);
    write(uint(data.externalForce.size()));
    for (const auto &value : data.externalForce) write(value);
    write(uint(data.memberInputKinds.size()));
    for (const auto &value : data.memberInputKinds) write(value);
    write(uint(data.memberInputValues.size()));
    for (const auto &value : data.memberInputValues) write(value);
    write(uint(data.portVoltages.size()));
    for (const auto &value : data.portVoltages) write(value);
    write(uint(data.portCharges.size()));
    for (const auto &value : data.portCharges) write(value);
    write(uint(data.portEnthalpies.size()));
    for (const auto &value : data.portEnthalpies) write(value);
    write(data.storedEnergy);
    write(data.kineticEnergy);
    write(data.initialTotalEnergy);
    write(data.cumulativeExternalWork);
    write(data.cumulativeSupportWork);
    write(data.cumulativeElectricalWork);
    write(data.cumulativeRestLengthWork);
}


/*********************************************************************
 * Version 1.3.21 local native control persistence                    *
 *********************************************************************/
namespace {
constexpr uint nativeCatalogBound=4096;
constexpr uint nativeEnvelopeBound=16u*1024u*1024u;
void nativeReadAscii(RFile &file, uint &value)
{
    const QString token=trussReadAsciiToken(file); bool ok=false; value=token.toUInt(&ok);
    if(!ok || token!=QString::number(value)) trussReadError();
}
void nativeReadAscii(RFile &file, quint64 &value)
{
    const QString token=trussReadAsciiToken(file); bool ok=false; value=token.toULongLong(&ok);
    if(!ok || token!=QString::number(value)) trussReadError();
}
template<class T> void nativeReadAscii(RFile &file,T &value) { trussReadAscii(file,value); }
void nativeReadAscii(RFile &file,QByteArray &value)
{
    QTextStream &stream=file.getTextStream(); stream.skipWhiteSpace(); QChar c; stream >> c;
    if(stream.status()!=QTextStream::Ok || c!='"') trussReadError();
    QByteArray hex;
    while(true) {
        stream >> c; if(stream.status()!=QTextStream::Ok) trussReadError();
        if(c=='"') break;
        if(!((c>='0'&&c<='9')||(c>='a'&&c<='f')) || hex.size()>=qint64(nativeEnvelopeBound)*2) trussReadError();
        hex.append(char(c.unicode()));
    }
    if(hex.size()%2) trussReadError(); value=QByteArray::fromHex(hex);
}
template<class T> void nativeReadBinary(RFile &file,T &value) { trussReadBinary(file,value); }
void nativeReadBinary(RFile &file,QByteArray &value)
{
    uint size=0; trussReadBinary(file,size);
    if(size>nativeEnvelopeBound || file.bytesAvailable()<qint64(size)) trussReadError();
    value=file.read(size); if(value.size()!=qint64(size) || file.error()!=RFile::NoError) trussReadError();
}
void nativeWriteAscii(RSaveFile &file,quint64 value,bool newline) { RFileIO::writeAscii(file,QString::number(value),newline); }
template<class T> void nativeWriteAscii(RSaveFile &file,const T &value,bool newline) { trussWriteAsciiValue(file,value,newline); }
void nativeWriteBinary(RSaveFile &file,quint64 value)
{
    if(file.write(reinterpret_cast<const char*>(&value),sizeof(value))!=qint64(sizeof(value)) || file.error()!=RFile::NoError)
        throw RError(RError::Type::WriteFile,R_ERROR_REF,"Failed to write native control counter.");
}
template<class T> void nativeWriteBinary(RSaveFile &file,const T &value) { trussWriteBinaryValue(file,value); }
// Enum values are checked as full-width integers before casting; otherwise
// 256 could alias a valid quint8 catalog value.
template<class IO,class Enum> void nativeEnum(IO &io,Enum &value,bool reading,uint mask)
{
    uint raw=uint(value); io(raw);
    if(reading) { if(raw>=32 || !(mask&(1u<<raw))) trussReadError(); value=static_cast<Enum>(raw); }
}
template<class IO,class T,class Visit> void nativeVector(IO &io,std::vector<T> &values,bool reading,Visit visit)
{
    uint size=uint(values.size()); io(size); if(size>nativeCatalogBound) trussReadError();
    if(reading) values.resize(size); for(auto &value:values) visit(value);
}
template<class IO> void nativeLimit(IO &io,double &value,bool reading,double absent)
{
    bool present=std::isfinite(value); io(present);
    if(present) io(value); else if(reading) value=absent;
}
template<class IO> void nativeSpecIO(IO &io,RStructuralNativeControlSpec &s,bool reading)
{
    io(s.schemaVersion); nativeEnum(io,s.source,reading,3);
    io(s.providerId); io(s.providerRevision); io(s.implementationSha256); io(s.dependencyEnvironmentManifest); io(s.dependencyEnvironmentSha256);
    io(s.controlEveryFESteps); io(s.samplePhase); io(s.deterministicSeed);
    nativeVector(io,s.sensors,reading,[&](auto &v) {
        io(v.sensorId); nativeEnum(io,v.quantity,reading,127); io(v.physicalNodeId); io(v.physicalMemberId); io(v.portId); io(v.axis);
    });
    nativeVector(io,s.actions,reading,[&](auto &v) {
        io(v.actionId); nativeEnum(io,v.quantity,reading,51); io(v.physicalNodeId); io(v.physicalMemberId); io(v.portId); io(v.axis);
        nativeLimit(io,v.minimum,reading,-std::numeric_limits<double>::infinity());
        nativeLimit(io,v.maximum,reading,std::numeric_limits<double>::infinity());
        nativeLimit(io,v.rateLimit,reading,std::numeric_limits<double>::infinity());
        io(v.initialCommand); io(v.delayTicks); io(v.holdTicks); io(v.noiseAmplitude);
    });
    nativeVector(io,s.piLoops,reading,[&](auto &v) { io(v.actionId); io(v.sensorId); io(v.referenceValue); io(v.proportionalGain); io(v.integralGain); io(v.bias); io(v.initialIntegral); });
}
template<class IO> void nativeStateIO(IO &io,RStructuralNativeControlState &s,bool reading)
{
    io(s.available); io(s.schemaVersion); io(s.sourceFingerprint); io(s.providerRevision); io(s.completedFE); io(s.completedControlTicks); io(s.fePhase); io(s.samplePhase);
    io(s.lastControllerInputState); io(s.controllerState);
    auto &f=s.lastFrame; io(f.completedFE); io(f.controlTick); io(f.fePhase); io(f.samplePhase); io(f.sampleTime); io(f.nextFETime); io(f.nextControlTime); io(f.physicalDt);
    nativeVector(io,f.samples,reading,[&](auto &v) { io(v.sensorId); nativeEnum(io,v.quantity,reading,127); io(v.value); });
    nativeVector(io,s.lastActions,reading,[&](auto &v) { io(v.actionId); nativeEnum(io,v.quantity,reading,51); io(v.previousApplied); io(v.requested); io(v.conditioned); io(v.applied); io(v.limitFlags); });
    nativeVector(io,s.heldValues,reading,[&](double &v) { io(v); });
    nativeVector(io,s.lastAppliedValues,reading,[&](double &v) { io(v); });
}
template<class T> bool nativeWriteVersion(RSaveFile &file,const T &data)
{
    data.validate();
    if(file.getVersion()>RVersion(1,3,20)) return true;
    if(data.hasNonDefaultData()) throw RError(RError::Type::WriteFile,R_ERROR_REF,"Native control v3 requires model format 1.3.21.");
    return false;
}
}

void RFileIO::readAscii(RFile &inFile,RStructuralNativeControlSpec &data)
{
    RStructuralNativeControlSpec candidate;
    if(inFile.getVersion()>RVersion(1,3,20)) {
        auto read=[&](auto &value) { nativeReadAscii(inFile,value); };
        nativeSpecIO(read,candidate,true); candidate.validate();
    }
    data=std::move(candidate);
}

void RFileIO::readBinary(RFile &inFile,RStructuralNativeControlSpec &data)
{
    RStructuralNativeControlSpec candidate;
    if(inFile.getVersion()>RVersion(1,3,20)) {
        auto read=[&](auto &value) { nativeReadBinary(inFile,value); };
        nativeSpecIO(read,candidate,true); candidate.validate();
    }
    data=std::move(candidate);
}

void RFileIO::writeAscii(RSaveFile &outFile,const RStructuralNativeControlSpec &data,bool addNewLine)
{
    if(!nativeWriteVersion(outFile,data)) return;
    RStructuralNativeControlSpec candidate=data;
    const int precision=outFile.getTextStream().realNumberPrecision();
    outFile.getTextStream().setRealNumberPrecision(std::numeric_limits<double>::max_digits10);
    auto write=[&](auto &value) { nativeWriteAscii(outFile,value,addNewLine); if(!addNewLine) RFileIO::writeAscii(outFile,' ',false); };
    try { nativeSpecIO(write,candidate,false); }
    catch(...) { outFile.getTextStream().setRealNumberPrecision(precision); throw; }
    outFile.getTextStream().setRealNumberPrecision(precision);
}

void RFileIO::writeBinary(RSaveFile &outFile,const RStructuralNativeControlSpec &data)
{
    if(!nativeWriteVersion(outFile,data)) return;
    RStructuralNativeControlSpec candidate=data;
    auto write=[&](auto &value) { nativeWriteBinary(outFile,value); };
    nativeSpecIO(write,candidate,false);
}

void RFileIO::readAscii(RFile &inFile,RStructuralNativeControlState &data)
{
    RStructuralNativeControlState candidate;
    if(inFile.getVersion()>RVersion(1,3,20)) {
        auto read=[&](auto &value) { nativeReadAscii(inFile,value); };
        nativeStateIO(read,candidate,true); candidate.validate();
    }
    data=std::move(candidate);
}

void RFileIO::readBinary(RFile &inFile,RStructuralNativeControlState &data)
{
    RStructuralNativeControlState candidate;
    if(inFile.getVersion()>RVersion(1,3,20)) {
        auto read=[&](auto &value) { nativeReadBinary(inFile,value); };
        nativeStateIO(read,candidate,true); candidate.validate();
    }
    data=std::move(candidate);
}

void RFileIO::writeAscii(RSaveFile &outFile,const RStructuralNativeControlState &data,bool addNewLine)
{
    if(!nativeWriteVersion(outFile,data)) return;
    RStructuralNativeControlState candidate=data;
    const int precision=outFile.getTextStream().realNumberPrecision();
    outFile.getTextStream().setRealNumberPrecision(std::numeric_limits<double>::max_digits10);
    auto write=[&](auto &value) { nativeWriteAscii(outFile,value,addNewLine); if(!addNewLine) RFileIO::writeAscii(outFile,' ',false); };
    try { nativeStateIO(write,candidate,false); }
    catch(...) { outFile.getTextStream().setRealNumberPrecision(precision); throw; }
    outFile.getTextStream().setRealNumberPrecision(precision);
}

void RFileIO::writeBinary(RSaveFile &outFile,const RStructuralNativeControlState &data)
{
    if(!nativeWriteVersion(outFile,data)) return;
    RStructuralNativeControlState candidate=data;
    auto write=[&](auto &value) { nativeWriteBinary(outFile,value); };
    nativeStateIO(write,candidate,false);
}
