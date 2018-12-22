//
// Created by vitalya on 18.12.18.
//

#include "TreeWidgetItem.h"

#include <QDebug>
#include <QRegExp>

static const QRegExp RegExp("(\\d+)");

TreeWidgetItem::TreeWidgetItem()
        : QTreeWidgetItem()
{
}

bool TreeWidgetItem::operator<(const QTreeWidgetItem& other) const {
    int lhs = 0;
    int rhs = 0;
    try {
        lhs = this->text(1).toInt();
    } catch (std::exception& e) {
        return false;
    }
    try {
        rhs = other.text(1).toInt();
    } catch (std::exception& e) {
        return false;
    }
    return lhs < rhs;
}

void TreeWidgetItem::SetNumber(int column, int number) {
    switch (column) {
        case 0:
            setText(column, text(column).replace(RegExp, QString::number(number)));
            break;
        case 1:
            setText(column, QString::number(number));
            break;
        default:
            throw std::logic_error("Wrong column index");
    }
}
