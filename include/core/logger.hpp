#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <core/async_logger.h>
namespace labelimg::core::logger {

using MutexAsyncLogger = AsyncLogger<queue::MutexPolicy>;
using CoroutineAsncLogger = AsyncLogger<queue::CoroutinePolicy>;





} // namespace labelimg::core::logger 

#endif // LOGGER_HPP