/* Copyright (c) 2012, STANISLAW ADASZEWSKI
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of STANISLAW ADASZEWSKI nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL STANISLAW ADASZEWSKI BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#ifndef QNEPORT_H
#define QNEPORT_H

//----------------------------------------------------------------------------------------------------------------------
/// @class QNEBlock
/// @brief This class originally written by STANISLAW ADASZEWSKI has been modified
/// @brief to be used for a node based user interface for creating OSL shaders.
/// @brief Original source can be found at http://algoholic.eu/qnodeseditor-qt-nodesports-based-data-processing-flow-editor/
/// @brief The parts modified will be specified
/// @author STANISLAW ADASZEWSKI modified by Declan Russell
/// @date Modifed 18/03/2015
//----------------------------------------------------------------------------------------------------------------------

#include <QGraphicsPathItem>

class QNEBlock;
class QNEConnection;

class QNEPort : public QGraphicsPathItem
{
public:
	enum { Type = QGraphicsItem::UserType + 1 };
	enum { NamePort = 1, TypePort = 2 };
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief enumorator to hold the variable type of the port
    /// @brief These are used becuase OsoReaders Types are in global name space and clash wish Qt so I have type cast them
    /// @brief to something else now. Bit of a hack :(
    /// @todo Get toby to romove is types from global name space so this class can just use OsoReader::Type
    //----------------------------------------------------------------------------------------------------------------------
    enum variableType{TypeInt,TypeFloat,TypeString, TypeColour, TypeMatrix, TypeNormal, TypePoint, TypeVoid, TypeVector};
    //----------------------------------------------------------------------------------------------------------------------

    QNEPort(QGraphicsItem *parent = 0);
	~QNEPort();

	void setNEBlock(QNEBlock*);
	void setName(const QString &n);
	void setIsOutput(bool o);
	int radius();
	bool isOutput();
	QVector<QNEConnection*>& connections();
	void setPortFlags(int);

	const QString& portName() const { return name; }
	int portFlags() const { return m_portFlags; }

	int type() const { return Type; }
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief mutator for the variable type of the port
    /// @brief added by Declan Russell
    //----------------------------------------------------------------------------------------------------------------------
    inline void setVaribleType(variableType _type){m_varibleType = _type;}
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief accessor to the varible type of the port
    /// @brief added by Declan Russell
    //----------------------------------------------------------------------------------------------------------------------
    inline variableType getVaribleType(){return m_varibleType;}
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief accessor to the port name
    /// @brief added by Declan Russell
    //----------------------------------------------------------------------------------------------------------------------
    inline QString getName(){return name;}
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief set initial paramiters string
    /// @brief added by Declan Russell
    //----------------------------------------------------------------------------------------------------------------------
    void setInitParams(std::vector<std::string> _initParams);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief accessor to initial paramiters to port
    //----------------------------------------------------------------------------------------------------------------------
    inline std::vector<std::string> getInitParams(){return m_initParams;}
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief accessor to the the ports label text
    //----------------------------------------------------------------------------------------------------------------------
    inline QString getLabelText(){return label->toPlainText();}
    //----------------------------------------------------------------------------------------------------------------------

	QNEBlock* block() const;

	quint64 ptr();
	void setPtr(quint64);

	bool isConnected(QNEPort*);

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
	QNEBlock *m_block;
	QString name;
	bool isOutput_;
	QGraphicsTextItem *label;
	int radius_;
	int margin;
	QVector<QNEConnection*> m_connections;
	int m_portFlags;
	quint64 m_ptr;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief a member to store the varible type of the port
    /// @brief added by Declan Russell
    //----------------------------------------------------------------------------------------------------------------------
    variableType m_varibleType;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Initial paramiters string vector
    /// @brief added by Declan Russell
    //----------------------------------------------------------------------------------------------------------------------
    std::vector<std::string> m_initParams;
    //----------------------------------------------------------------------------------------------------------------------
};

#endif // QNEPORT_H
