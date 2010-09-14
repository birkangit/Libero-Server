/*
* copyright (c) 2010 Sveriges Television AB <info@casparcg.com>
*
*  This file is part of CasparCG.
*
*    CasparCG is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    CasparCG is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.

*    You should have received a copy of the GNU General Public License
*    along with CasparCG.  If not, see <http://www.gnu.org/licenses/>.
*
*/
 
#pragma once

#include "CLKCommand.h"
#include "../../../common/io/ProtocolStrategy.h"
#include "../../renderer/render_device.h"

namespace caspar { namespace CLK {

class CLKProtocolStrategy : public caspar::IO::IProtocolStrategy
{
public:
	CLKProtocolStrategy(const std::vector<renderer::render_device_ptr>& channels);

	void Parse(const TCHAR* pData, int charCount, caspar::IO::ClientInfoPtr pClientInfo);
	UINT GetCodepage() { return 28591; }	//ISO 8859-1
	
private:
	enum ParserState
	{
		ExpectingNewCommand,
		ExpectingCommand,
		ExpectingClockID,
		ExpectingTime,
		ExpectingParameter
	};

	ParserState	currentState_;
	CLKCommand currentCommand_;
	std::wstringstream currentCommandString_;

	renderer::render_device_ptr pChannel_;

	bool bClockLoaded_;
};

}}