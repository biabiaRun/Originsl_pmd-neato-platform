* {
    outline: none;
    border:none;
}

*:focus {
    outline: none;
    border:none;
}

QListView::item[separator='true'] {
    border-bottom-width:1px;
    border-bottom-style:solid;
    border-bottom-color:qlineargradient(
    x1: 0, y1: 0,
    x2: 1, y2: 0,
    stop: 0.1 #348bbb,
    stop: 0.5 #0c62a0,
    stop: 0.9 #348bbb);
}

QListView::item:selected[separator='true'] {
    border-bottom-width:1px;
    border-bottom-style:solid;
    border-bottom-color:qlineargradient(
    x1: 0, y1: 0,
    x2: 1, y2: 0,
    stop: 0.1 #348bbb,
    stop: 0.5 #ffffff,
    stop: 0.9 #348bbb);
}

QToolTip {
    font-size:14pt;
}

QListView::item:selected {
    background-color:transparent;
    color:white;
    border:1px;
}

QListView::item {
    background-color:transparent;
    color:black;
    border:1px;
}

QScrollBar::handle:vertical {
    border: 1px solid black;
    background: white;
    min-height: 40px;
    margin: 0px 0px 0px 0px;
}

QScrollBar::add-line:vertical {
    height: 0px;
    subcontrol-position: bottom;
    subcontrol-origin: margin;
    border: none;
}

QScrollBar::sub-line:vertical {
    height: 0px;
    subcontrol-position: top;
    subcontrol-origin: margin;
    border: none;
}

QScrollBar::handle:horizontal {
    border: 1px solid black;
    background: white;
    min-width: 40px;
    margin: 0px 0px 0px 0px;
}

QScrollBar::add-line:horizontal {
    width: 0px;
    subcontrol-position: bottom;
    subcontrol-origin: margin;
    border: none;
}

QScrollBar::sub-line:horizontal {
    width: 0px;
    subcontrol-position: top;
    subcontrol-origin: margin;
    border: none;
}

QMenu {
    background-color: rgb(52, 139, 187);
    border: 1px solid white;
}

QMenu::item {
    padding: 2px 25px 2px 20px;
    background: rgb(52, 139, 187);
    color: rgb(220, 220, 220);
}

QMenu::item:selected {
    background: rgb(35, 93, 125);
    color:white;
}

QTextEdit {
    background:rgba(0,0,0,0.1);
    color:white;
}

QComboBox {
    background-color: #ABABAB;
    border: 1px solid black;
}

QTabWidget::pane:selected {
    border: 1px solid white;
    background-color: rgb(52, 139, 187);
    color:white;
}

QTabBar::tab {
    background-color: rgb(35, 93, 125);
    color:white;
    margin-right: 5px;
    margin-bottom: 7px;
}

QTabBar::tab::hover {
    background-color: rgb(255, 255, 255, 100);
}

QTabBar::tab:selected {
    border: 1px solid white;
    background-color: rgb(52, 139, 187);
    color:white;
}