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

#ifndef GRAPH_H
#define GRAPH_H

#include <QPainter>
#include <QRect>
#include <QFont>

class GraphItem;

class Graph
{
public:
    Graph();
    void SetSize(QRect size);
    void AddItem(GraphItem *item);
    void Draw(QPainter &painter);

    int marginl,marginb,marginr,margint;
    int xo,yo,w,h;

private:
    std::vector<GraphItem *> items;
};

class GraphItem
{
public:
    GraphItem(Graph *g);
    virtual ~GraphItem() {};
    virtual void Draw(QPainter &painter) = 0;

    bool enabled;
    QPen pen;

protected:
    Graph *graph;
};

class GraphScale : public GraphItem
{
public:
    enum pos_t {pos_left,pos_right,pos_bottom,pos_top};

    GraphScale(Graph *g, pos_t p);
    virtual ~GraphScale() {};
    void Draw(QPainter &painter);
    void SetIncAuto();
    void SetMinAuto();
    void Expand(double min,double max);

    QFont font;
    double vmin, vmax, vinc;
    pos_t pos;
    int labdps;
    QString title,labsuffix;
    double labdiv;
};

class GraphDataItem : public GraphItem
{
public:
    GraphDataItem(Graph *g, GraphScale *s);

protected:
    GraphScale *scale;
};

class GraphHorizLine : public GraphDataItem
{
public:
    GraphHorizLine(Graph *g, GraphScale *s) : GraphDataItem(g,s) {};
    virtual ~GraphHorizLine() {};
    void Draw(QPainter &painter);

    double val;

    //private:
    //    GraphScale *scale;
};

class GraphVertLine : public GraphDataItem
{
public:
    GraphVertLine(Graph *g, GraphScale *s) : GraphDataItem(g,s) {};
    virtual ~GraphVertLine() {};
    void Draw(QPainter &painter);

    double val;

//private:
//    GraphScale *scale;
};

class GraphTrace : public GraphDataItem
{
public:
    GraphTrace(Graph *g, GraphScale *s) : GraphDataItem(g,s) {};
    virtual ~GraphTrace() {};
    void Draw(QPainter &painter);

    std::vector<double> points;

//private:
//    GraphScale *scale;
};

#endif // GRAPH_H
