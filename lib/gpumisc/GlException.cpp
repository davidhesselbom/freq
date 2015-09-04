#include "GlException.h"
#include "gluerrorstring.h"

#include "gl.h"
#include "backtrace.h"
#include "log.h"

#include <boost/format.hpp>


void GlException::check_error( ) {
	check_error( glGetError() );
}


void GlException::check_error( GLenum errorCode ) {
    if (GL_NO_ERROR != errorCode) {
        GlException x; x
                << GlException_namedGlError(errorCode)
                << GlException_namedGlErrorString(
                       str(boost::format("errorCode=0x%X: %s")
                           % errorCode
                           % (const char*)gluErrorString( errorCode )))
                << Backtrace::make(2);

        if (std::uncaught_exception())
            Log("Ignoring GlException\n%s") % boost::diagnostic_information(x);
        else
            BOOST_THROW_EXCEPTION (x);
	}
}


void GlException::check_error( const char* functionMacro, 
	                     const char* fileMacro, int lineMacro,
						 const char* callerMessage) {
	check_error( glGetError(), functionMacro, fileMacro, 
		              lineMacro, callerMessage );
}


void GlException::check_error( GLenum errorCode, const char* functionMacro, 
		                     const char* fileMacro, int lineMacro, 
							 const char* callerMessage ) {
	if (GL_NO_ERROR != errorCode) {
        // Reset OpenGL error state as we handle this error with exceptions from here on
        GLenum cleared_error = glGetError();
		std::string context_error;
		if (cleared_error == GL_INVALID_OPERATION)
		{
            context_error = "\nNo OpenGL context is currently active";
		}

        GlException x; x
                    << GlException_namedGlError(errorCode)
                    << GlException_namedGlErrorString(
                           str(boost::format("errorCode=0x%X: %s")
                               % errorCode
                               % (const char*)gluErrorString( errorCode )))
                    << GlException_message((callerMessage?callerMessage:"") + context_error)
                    << Backtrace::make(2);

        if (std::uncaught_exception())
            Log("Throw cancelled due to previous uncaught_exception");
        else try
        {
            ::boost::exception_detail::throw_exception_(x,
                                                        functionMacro,
                                                        fileMacro,
                                                        lineMacro);
        } catch (boost::exception& x) {
            Log("Throwing GlException\n%s") % boost::diagnostic_information(x);
            throw;
        }
    }
}
