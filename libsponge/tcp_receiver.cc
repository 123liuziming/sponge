#include "tcp_receiver.hh"

#include <iostream>

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    //waiting
    if (!ackno().has_value() && !seg.header().syn) {
        return;
    }
    //连接已经关闭了
    if (_reassembler.stream_out().input_ended()) {
        return;
    }
    // syn代表建立连接
    if (seg.header().syn && !ackno().has_value()) {
        _isn = seg.header().seqno;
    }

    auto index = unwrap(WrappingInt32(seg.header().seqno - _offset), _isn.value(), _reassembler.stream_out().bytes_written());

    if (index >= _reassembler.get_index_now() + window_size()) {
        return;
    }

    if (ackno().has_value() && !_reassembler.stream_out().input_ended()) {
        auto payload = seg.payload().copy();
        _reassembler.push_substring(payload, index, seg.header().fin);
    }

    if (seg.header().syn) {
        ++_offset;
    }

    if (_reassembler.stream_out().input_ended()) {
        ++_offset;
    }

}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (_isn.has_value()) {
        return wrap(_reassembler.get_index_now() + _offset, _isn.value());
    }
    return {};
}

size_t TCPReceiver::window_size() const {
    return _reassembler.stream_out().remaining_capacity();
}

std::optional<WrappingInt32> TCPReceiver::isn() {
    return _isn;
}

size_t TCPReceiver::capacity() {
    return _capacity;
}
