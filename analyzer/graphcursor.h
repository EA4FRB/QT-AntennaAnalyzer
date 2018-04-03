/*
(C) Copyright 2015 Jeremy Burton

This file is part of Sark-100-antenna-analyzer.

Sark-100-antenna-analyzer is free software: you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Sark-100-antenna-analyzerr is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GRAPHCURSOR_H
#define GRAPHCURSOR_H

#include <QWidget>
#include <QPainter>

class GraphCursor : public QWidget
{
    Q_OBJECT
public:
    explicit GraphCursor(QWidget *parent = 0);

signals:

public slots:

private:
    void paintEvent(QPaintEvent *);
};

#endif // GRAPHCURSOR_H
