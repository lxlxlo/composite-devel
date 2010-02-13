/*
 * Copyright(c) 2010 by Gabriel M. Beddingfield <gabriel@teuton.org>
 *
 * This file is part of Tritium
 *
 * Tritium is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tritium is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <Tritium/Mixer.hpp>
#include "MixerPrivate.hpp"
#include <cstring> // memcpy
#include <algorithm>
#include <cmath>

using namespace Tritium;

////////////////////////////////////////////////////////////
// Mixer
////////////////////////////////////////////////////////////

Mixer::Mixer(uint32_t max_buffer)
{
    d = new MixerPrivate();
    d->_max_buf = max_buffer;
}

Mixer::~Mixer()
{
    delete d;
    d = 0;
}

T<AudioPort>::shared_ptr Mixer::allocate_port(
    const QString& name,
    AudioPort::flow_t /*in_or_out*/,
    AudioPort::type_t type,
    uint32_t /*size*/)
{
    T<Mixer::Channel>::shared_ptr tmp(new Mixer::Channel);
    tmp->gain( 1.0f );
    if( type == AudioPort::MONO ) {
	tmp->port() = d->new_mono_port();
	tmp->pan_L( 0.5f );
    } else {
	assert(type == AudioPort::STEREO);
	tmp->port() = d->new_stereo_port();
	tmp->pan_L( 0.0f );
	tmp->pan_R( 1.0f );
    }
    QMutexLocker lk(&d->_in_ports_mutex);
    d->_in_ports.push_back(tmp);
    return tmp->port();
}

void Mixer::release_port(T<AudioPort>::shared_ptr port)
{
    d->delete_port(port);
}

static void set_zero_flag_fun(T<Mixer::Channel>::shared_ptr x) {
    if(x && x->port()) x->port()->set_zero_flag(true);
}

void Mixer::pre_process()
{
    std::for_each(d->_in_ports.begin(), d->_in_ports.end(), set_zero_flag_fun);
}

void Mixer::mix_send_return(uint32_t nframes)
{
    #warning "TO-DO"
}

void Mixer::mix_down(uint32_t nframes, float* left, float* right)
{
    #warning "This is the prosaic approach.  Need an optimized one."
    MixerPrivate::port_list_t::iterator it;
    bool zero = true;

    /* See below for
     * the "Theory of Pan"
     */
    for(it=d->_in_ports.begin() ; it!=d->_in_ports.end() ; ++it) {
	Channel& chan = **it;
	T<AudioPort>::shared_ptr port = chan.port();
	if( port->zero_flag() ) continue;
	if( port->type() == AudioPort::MONO ) {
	    float gL, gR, pan, gain;
	    gain = chan.gain();
	    pan = chan.pan();
	    MixerPrivate::eval_pan(gain, pan, gL, gR);
	    if(zero) {
		MixerPrivate::copy_buffer_with_gain(left, port->get_buffer(), nframes, gL);
		MixerPrivate::copy_buffer_with_gain(right, port->get_buffer(), nframes, gR);
	    } else {
		MixerPrivate::mix_buffer_with_gain(left, port->get_buffer(), nframes, gL);
		MixerPrivate::mix_buffer_with_gain(right, port->get_buffer(), nframes, gR);
	    }
	} else {
	    assert( port->type() == AudioPort::STEREO );
	    float gL, gR, pan, gain;

	    // Left
	    gain = chan.gain();
	    pan = chan.pan_L();
	    MixerPrivate::eval_pan(gain, pan, gL, gR);
	    if(zero) {
		MixerPrivate::copy_buffer_with_gain(left, port->get_buffer(), nframes, gL);
		MixerPrivate::copy_buffer_with_gain(right, port->get_buffer(), nframes, gR);
	    } else {
		MixerPrivate::mix_buffer_with_gain(left, port->get_buffer(), nframes, gL);
		MixerPrivate::mix_buffer_with_gain(right, port->get_buffer(), nframes, gR);
	    }
	    pan = chan.pan_R();
	    MixerPrivate::eval_pan(gain, pan, gL, gR);
	    MixerPrivate::mix_buffer_with_gain(left, port->get_buffer(1), nframes, gL);
	    MixerPrivate::mix_buffer_with_gain(right, port->get_buffer(1), nframes, gR);
	}
	zero = false;
    }
    if(zero) {
	memset(left, 0, nframes * sizeof(float));
	memset(right, 0, nframes * sizeof(float));
    }
}

size_t Mixer::count()
{
    return d->_in_ports.size();
}

T<AudioPort>::shared_ptr Mixer::port(size_t n)
{
    assert( n < d->_in_ports.size() );
    return d->_in_ports[n]->port();
}

T<Mixer::Channel>::shared_ptr Mixer::channel(size_t n)
{
    assert( n < d->_in_ports.size() );
    return d->_in_ports[n];
}

T<Mixer::Channel>::shared_ptr Mixer::channel(const T<AudioPort>::shared_ptr port)
{
    return d->channel_for_port(port);
}

////////////////////////////////////////////////////////////
// MixerPrivate
////////////////////////////////////////////////////////////

MixerPrivate::port_ref_t MixerPrivate::new_mono_port()
{
    port_ref_t tmp( new AudioPortImpl(AudioPort::MONO, _max_buf) );
    return boost::dynamic_pointer_cast<AudioPort>(tmp);
}

MixerPrivate::port_ref_t MixerPrivate::new_stereo_port()
{
    port_ref_t tmp( new AudioPortImpl(AudioPort::STEREO, _max_buf) );
    return boost::dynamic_pointer_cast<AudioPort>(tmp);
}

void MixerPrivate::delete_port(MixerPrivate::port_ref_t port)
{
    port_list_t::iterator it;
    it = find(_in_ports.begin(), _in_ports.end(), port);
    QMutexLocker lk( &_in_ports_mutex );
    _in_ports.erase(it);
}

T<Mixer::Channel>::shared_ptr MixerPrivate::channel_for_port(const MixerPrivate::port_ref_t port)
{
    for(size_t k; k<_in_ports.size() ; ++k) {
	if( _in_ports[k]->port() == port ) return _in_ports[k];
    }
    return T<Mixer::Channel>::shared_ptr();
}

/**
 * Evaluate the pan and gain settings into a left and right gain setting.
 *
 * Theory of Pan.
 *
 * Pan is the position of average intensity of sound.  Thus:
 *
 *   + When sound is in the center, Pan = 0.5
 *   + When sound is full left,     Pan = 0.0
 *   + When sound is full right,    Pan = 1.0
 *
 * Thus:
 *
 *    Pan = Right / (Left + Right)       [Eqn. 1]
 *
 * And:
 *
 *    (1-Pan) = Left / (Left + Right)    [Eqn. 2]
 *
 * When a "master gain" is involved... we modify it like this:
 *
 *    Gain = max(Left, Right)
 *
 * So, when Left > Right, the Gain = Left.  When Right > Left,
 * Gain = Right.
 *
 * When Pan <= .5 (Left > Right):
 *
 *    Pan = Right / (Gain + Right)        [Eqn. A]
 *
 * When Pan >= .5 (Right < Left):
 *
 *    Pan = Gain / (Left + Gain)          [Eqn. B]
 *
 * Simplifying Eqn. A looks like this:
 *
 *    Pan = Right / (Gain + Right)
 *    Pan * (Gain + Right) = Right
 *    Pan * Gain + Pan * Right = Right
 *    Pan * Gain = Right - Pan * Right
 *    Pan * Gain = (1-Pan)*Right
 *    Right = Pan * Gain / (1 - Pan)
 *    Left = Gain
 *
 * Simplifying Eqn. B looks like this:
 *
 *    Pan = Gain / (Left + Gain)
 *    Pan * (Left + Gain) = Gain
 *    Pan * Left + Pan * Gain = Gain
 *    Pan * Left = Gain - (Pan * Gain)
 *    Pan * Left = Gain * (1-Pan)
 *    Left = Gain * (1-Pan) / Pan
 *    Right = Gain
 *
 */
void MixerPrivate::eval_pan(float gain, float pan, float& left, float& right)
{
    float L = 0.0f, R = 0.0f;
    if( (pan > 1.0f) || (pan < 0.0f) ) {
	left = L;
	right = R;
	return;
    }

    if(pan < .5) {
	R = pan * gain / (1.0f - pan);
	L = gain;
    } else {
	L = gain * (1.0f - pan) / pan;
	R = gain;
    }
    if( gain > 1.0e-6 ) assert( ::fabs(pan - (R / (R+L))) < 1.0e-6 );
    left = L;
    right = R;
}

void MixerPrivate::copy_buffer_no_gain(float* dst, float* src, uint32_t nframes)
{
    memcpy(dst, src, nframes * sizeof(float));
}

void MixerPrivate::copy_buffer_with_gain(float* dst, float* src, uint32_t nframes, float gain)
{
    mult_gain t;
    t.gain = gain;
    std::transform(src, src+nframes, dst, t);
}

void MixerPrivate::mix_buffer_no_gain(float* dst, float* src, uint32_t nframes)
{
    std::transform(src, src+nframes, dst, dst, std::plus<float>());
}

void MixerPrivate::mix_buffer_with_gain(float* dst, float* src, uint32_t nframes, float gain)
{
    add_with_gain t;
    t.gain = gain;
    std::transform(src, src+nframes, dst, dst, t);
}

////////////////////////////////////////////////////////////
// Mixer::Channel
////////////////////////////////////////////////////////////

Mixer::Channel::Channel()
{
    d = new ChannelPrivate();
}

Mixer::Channel::~Channel()
{
    delete d;
    d = 0;
}

Mixer::Channel::Channel(const Channel& c)
{
    d = new ChannelPrivate();
    (*d) = (*c.d);
}

Mixer::Channel& Mixer::Channel::operator=(const Channel& c)
{
    (*d) = (*c.d);
    return *this;
}

const T<AudioPort>::shared_ptr Mixer::Channel::port() const
{
    return d->_port;
}

T<AudioPort>::shared_ptr& Mixer::Channel::port()
{
    return d->_port;
}

float Mixer::Channel::gain() const
{
    return d->_gain;
}

void Mixer::Channel::gain(float gain)
{
    if(gain < 0.0f) {
	d->_gain = 0.0f;
    } else {
	d->_gain = gain;
    }
}

float Mixer::Channel::pan() const
{
    return pan_L();
}

void Mixer::Channel::pan(float pan)
{
    pan_L(pan);
}

float Mixer::Channel::pan_L() const
{
    return d->_pan_L();
}

void Mixer::Channel::pan_L(float pan)
{
    d->_pan_L(pan);
}

float Mixer::Channel::pan_R() const
{
    return d->_pan_R();
}

void Mixer::Channel::pan_R(float pan)
{
    d->_pan_R(pan);
}
