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

#pragma once

#include <string>
#include <sstream>

namespace NBase
{
  class Result
  {
  public:

    Result(const std::string &error) ;
    Result(const std::ostringstream &error) ;
    Result(Result &cause,const std::string &error) ;
    ~Result() ;
    Result(const Result &other) ;

    bool Failed() ;
    bool Succeeded() ;
    std::string GetDescription() ;

    Result &operator=(const Result &) ;

    static Result NoError ;
  protected:
    Result() ;
  private:
    bool success_ ;
    std::string error_ ;
    mutable bool checked_ ;
    mutable Result *child_ ;
  };
}

#define RETURN_IF_FAILED_MESSAGE(r,m) if (r.Failed()) { return NBase::Result(r,m) ; }
#define RETURN_IF_FAILED(r) if (r.Failed()) { return r ; }
#define LOG_IF_FAILED(r,m) if (r.Failed()) { NBase::Result __combined(r,m); Trace::Error(__combined.GetDescription().c_str()) ; __combined.Failed();}

