#pragma once

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <iostream>

class MyQGraphicsScene : public QGraphicsScene
{
	//Q_OBJECT
public:
	int _n;
	//typedef void(*callback_t)(QColor);
	//MainWindow * _main_window;

	MyQGraphicsScene()
	{
		//_main_window = NULL;
		_n = 8;
		std::cout << "MyQGraphicsScene created.\n";
	}

	//MyQGraphicsScene(int number_of_colours)
	//{
	//	n = number_of_colours;
	//	std::cout << "MyQGraphicsScene created by MyQGraphicsScene(int number_of_colours).\n";
	//}

	//void set_main_window(MainWindow * main_window)
	//{
	//	_main_window = main_window;
	//}

	void set_number_of_colours(int number_of_colours)
	{
		_n = number_of_colours;
	}

	~MyQGraphicsScene()
	{
		std::cout << "MyQGraphicsScene destroyed.\n";
	}

	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event)
	{
		QGraphicsScene::mousePressEvent(event);
		auto list = this->items();
		for (int i = 0; i < list.size(); i++)
		{
			auto selected = list.at(i)->isSelected();
			std::cout << i << (selected ? " selected" : " not selected") << std::endl;
			if (selected)
			{
				int index = list.at(i)->data(0).toInt();
				std::cout << "index=" << index << std::endl;
				//if (_main_window)
				{
					QColor colour;
					colour.setHsv(index * 360 / _n, 255, 255);
					//_main_window->optimise_transfer_function_for_colour(colour);
				}
			}
		}
	}
};
