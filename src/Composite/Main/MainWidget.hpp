/*
 * Copyright(c) 2010 by Gabriel M. Beddingfield <gabriel@teuton.org>
 *
 * This file is part of Composite
 *
 * Composite is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Composite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef COMPOSITE_MAIN_MAINWIDGET_HPP
#define COMPOSITE_MAIN_MAINWIDGET_HPP

#include <QtGui/QWidget>
#include <Tritium/memory.hpp>

class QPaintEvent;

namespace Tritium
{
    class Engine;
}

namespace Composite
{
namespace Main
{
    class MainWidgetPrivate;

    /**
     * \brief The central workspace for Composite
     */
    class MainWidget : public QWidget
    {
	Q_OBJECT
    private:
	MainWidgetPrivate * const _d;

    public:
	MainWidget(int argc = 0, char* argv[] = 0, QWidget *parent = 0);
	virtual ~MainWidget();

	void set_audio_engine( Tritium::Engine *eng );

    public slots:
	void go_matrix();
	void go_edit();

	void x_play();
	void x_stop();

    protected:
	virtual void paintEvent(QPaintEvent *event);

    };

} // namespace Main
} // namespace Composite

#endif // COMPOSITE_MAIN_MAINWIDGET_HPP
