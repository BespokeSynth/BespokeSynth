// Copyright (c) 2017 Ableton AG, Berlin
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "Result.h"
#include <assert.h>

using namespace NBase;

Result Result::NoError ;

Result::Result()
:success_(true)
,error_("success")
,checked_(true)
,child_(0){}

Result::Result(const std::string &error)
:success_(false)
,error_(error)
,checked_(false)
,child_(0) {}

Result::Result(const std::ostringstream &error)
:success_(false)
,error_(error.str())
,checked_(false)
,child_(0) {}

Result::Result(Result &cause,const std::string &error)
:success_(false)
,error_(error)
,checked_(false)
,child_(new Result(cause)) {
	cause.checked_ = true ;
  child_->checked_ = true;
}

Result::Result(const Result &other) {
	success_=other.success_ ;
	error_=other.error_ ;
	checked_ = false ;
	other.checked_ = true ;
	child_ = other.child_ ;
	other.child_ = 0 ;
}

Result::~Result() {
	assert(checked_) ;
	delete(child_) ;
}

Result &Result::operator =(const Result &other) {
	success_=other.success_ ;
	error_=other.error_ ;
	checked_ = false ;
	other.checked_ = true ;
	child_ = other.child_ ;
	other.child_=0 ;
	return *this;
}

bool Result::Failed() {
	checked_= true ;
	return !success_ ;
}

bool Result::Succeeded() {
	checked_= true ;
	return success_ ;
}

std::string Result::GetDescription() {
	std::string description = error_ ;
	if (child_) {
		description+="\n>>>> " ;
		description+=child_->GetDescription() ;
	}
	return description ;
}
