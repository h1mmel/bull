#ifndef _ASIO_TYPE_H_
#define _ASIO_TYPE_H_

#include "boost/asio.hpp"

namespace bull {
namespace types {

using IoContext = boost::asio::io_context;
using Work = IoContext::work;
using WorkPtr = std::unique_ptr<Work>;
using Tcp = boost::asio::ip::tcp;
using TcpSocket = Tcp::socket;
using TcpResolver = Tcp::resolver;
using ResultType = TcpResolver::results_type;
using EndPoint = Tcp::endpoint;

using StreamBuffer = boost::asio::streambuf;
using ConstBuffer = boost::asio::const_buffer;
using MutableBuffer = boost::asio::mutable_buffer;

using ErrorCode = boost::system::error_code;

}   // namespace types
}   // namespace bull

#endif  // _ASIO_TYPE_H_
