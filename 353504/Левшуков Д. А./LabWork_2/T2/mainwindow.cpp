#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ctrlo= new QShortcut(this);
    ctrlo->setKey(Qt::CTRL + Qt::Key_O);
    connect(ctrlo, SIGNAL(activated()), this, SLOT(on_OpenFileButton_clicked()));

    ctrls= new QShortcut(this);
    ctrls->setKey(Qt::CTRL + Qt::Key_S);
    connect(ctrls, SIGNAL(activated()), this, SLOT(on_SaveFileButton_clicked()));
    //showFullScreen();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_OpenFileButton_clicked()
{
    file=(QFileDialog::getOpenFileName(this,"Open File","/home/dzmitry","*.inf")).toUtf8().constData();
    qDebug()<<file;
    if(file=="")
        return;
    ui->FileBrowser->setText(QString::fromStdString(file));
    FileRead();
}

void MainWindow::FileRead()
{
    std::ifstream fin(file);
    std::string s;
    std::getline(fin,s);
    if(s!=orders_head)
    {
        //Bad file
        qDebug()<<"Bad file";
        return;
    }

    Clear();
    orders_in_file="";

    while(s!=couriers_head&&std::getline(fin,s))//Orders read
    {
        if(s==couriers_head)break;
        orders_in_file+=s+'\n';
        qDebug()<<"|ORDERS READ|"<<s;
        if(ReadOrders(s))
            order_position_in_file.push_back({orders_in_file.size()-s.size()-1,s.size()+1});

    }
    ui->OrderNumberSelect->addItems(select_orders);
    couriers_in_file="";

    while(std::getline(fin,s))
    {
        qDebug()<<"|COURIERS READ|"<<s;
        couriers_in_file+=s+'\n';
        if(ReadCouriers(s))
            courier_position_in_file.push_back({couriers_in_file.size()-s.size()-1,s.size()+1});
    }
    ui->CourierNumberSelect->addItems(select_couriers);
    DistributeOrders();
    //std::ofstream fout(file);
    //fin<<1;
}

bool MainWindow::ReadOrders(std::string s)
{
    //Example:[132332] {Belarus, Mogilev, blablabla, 54, 2} 24.01.2020 26.02.2022 1000 1400 228
    order now;
    Data from,to;

    if(s[0]!='[')return false;

    int n=s.size(),i=1;

    for(;i<n;i++)
    {
        if(s[i]==']')
            break;
    }
    if(i==n)return false;

    now.setNumber(s.substr(1,i-1));
    //qDebug()<<"1:"<<now.get_number()<<" "<<now.get_addres()<<" "<<now.get_date_from()<<" "<<now.get_date_to()<<" "<<now.getTime_from()<<" "<<now.getTime_to()<<" "<<now.get_weight();
    i+=2;
    if(s[i]!='{')return false;
    int j=i+1;

    for(;i<n;i++)
    {
        if(s[i]=='}')
            break;
    }
    if(i==n)return false;

    now.setAddres(s.substr(j,i-j));
    //qDebug()<<"2:"<<now.get_number()<<" "<<now.get_addres()<<" "<<now.get_date_from()<<" "<<now.get_date_to()<<" "<<now.getTime_from()<<" "<<now.getTime_to()<<" "<<now.get_weight();

    i+=2;
    j=i;
    for(;i<n;i++)
    {
        if(s[i]==' ')
        {
            i--;
            break;
        }
    }
    if(i==n)return false;
    //qDebug()<<"Try init:"<<s.substr(j,i-j+1);
    if(from.Init(s.substr(j,i-j+1)))
    {
        now.setDate_from(from);
    }
    else
        return false;
    //qDebug()<<"3:"<<now.get_number()<<" "<<now.get_addres()<<" "<<now.get_date_from()<<" "<<now.get_date_to()<<" "<<now.getTime_from()<<" "<<now.getTime_to()<<" "<<now.get_weight();

    i+=2;
    j=i;
    for(;i<n;i++)
    {
        if(s[i]==' ')
        {
            i--;
            break;
        }
    }
    if(i==n)return false;

    if(to.Init(s.substr(j,i-j+1)))
    {
        now.setDate_to(to);
    }
    else
        return false;
    //qDebug()<<"4:"<<now.get_number()<<" "<<now.get_addres()<<" "<<now.get_date_from()<<" "<<now.get_date_to()<<" "<<now.getTime_from()<<" "<<now.getTime_to()<<" "<<now.get_weight();

    i+=2;
    j=i;
    //qDebug()<<s[i];
    for(;i<n;i++)
    {
        if(s[i]==' ')
        {
            i--;
            break;
        }
    }
    //qDebug()<<i;
    if(i==n)return false;
    int t=0;
    try
    {
        t=stoi(s.substr(j,i-j+1));
    }
    catch(...)
    {
        qDebug()<<"stoi catch error";
        return false;
    }
    for(;j<=i;j++)
        if(!isdigit(s[j]))
            return false;

    now.setTime_from(t);

    //qDebug()<<"5:"<<now.get_number()<<" "<<now.get_addres()<<" "<<now.get_date_from()<<" "<<now.get_date_to()<<" "<<now.getTime_from()<<" "<<now.getTime_to()<<" "<<now.get_weight();

    i+=2;
    j=i;
    for(;i<n;i++)
    {
        if(s[i]==' ')
        {
            i--;
            break;
        }
    }
    if(i==n)return false;
    t=0;
    try
    {
        t=stoi(s.substr(j,i-j+1));
    }
    catch(...)
    {
        return false;
    }
    for(;j<=i;j++)
        if(!isdigit(s[j]))
            return false;

    now.setTime_to(t);
    //qDebug()<<"6:"<<now.get_number()<<" "<<now.get_addres()<<" "<<now.get_date_from()<<" "<<now.get_date_to()<<" "<<now.getTime_from()<<" "<<now.getTime_to()<<" "<<now.get_weight();
    i+=2;
    j=i;

    unsigned long long w=0;
    try
    {
        w=stoull(s.substr(j,n-j));
    }
    catch(...)
    {
        return false;
    }
    for(;j<n;j++)
        if(!isdigit(s[j]))
            return false;

    now.setWeight(w);
    //qDebug()<<"7:"<<now.get_number()<<" "<<now.get_addres()<<" "<<now.get_date_from()<<" "<<now.get_date_to()<<" "<<now.getTime_from()<<" "<<now.getTime_to()<<" "<<now.get_weight();
    if(to>=from)
    {
        qDebug()<<"Date to>=from";
    }
    else
        return false;
    if(now.getTime_to()>=now.getTime_from())
    {
        qDebug()<<"Time to>=from";
    }
    else return false;
    o.push_back(now);
    select_orders.push_back(QString::fromStdString(now.get_in_string()));
    qDebug()<<"FINAL:"<<now.get_number()<<" "<<now.get_addres()<<" "<<now.get_date_from()<<" "<<now.get_date_to()<<" "<<now.getTime_from()<<" "<<now.getTime_to()<<" "<<now.get_weight();
    return true;
}

bool MainWindow::ReadCouriers(std::string s)

{
    //Example:[132332] {Belarus, Mogilev, blablabla, 54, 2} 24.01.2020 26.02.2022 1000 1400 228
    courier now;

    if(s[0]!='[')return false;

    int n=s.size(),i=1;

    for(;i<n;i++)
    {
        if(s[i]==']')
            break;
    }
    if(i==n)return false;

    now.setNumber(s.substr(1,i-1));
    qDebug()<<"1:"<<now.getNumber()<<" "<<now.getName()<<" "<<now.getTime_from()<<" "<<now.getTime_to()<<" "<<now.getMax_weight();
    i+=2;
    if(s[i]!='{')return false;
    int j=i+1;

    for(;i<n;i++)
    {
        if(s[i]=='}')
            break;
    }
    if(i==n)return false;

    now.setName(s.substr(j,i-j));
    qDebug()<<"2:"<<now.getNumber()<<" "<<now.getName()<<" "<<now.getTime_from()<<" "<<now.getTime_to()<<" "<<now.getMax_weight();
    i+=2;
    j=i;
    //qDebug()<<s[i];
    for(;i<n;i++)
    {
        if(s[i]==' ')
        {
            i--;
            break;
        }
    }
    //qDebug()<<i;
    if(i==n)return false;
    int t=0;
    try
    {
        t=stoi(s.substr(j,i-j+1));
    }
    catch(...)
    {
        qDebug()<<"stoi catch error";
        return false;
    }
    for(;j<=i;j++)
        if(!isdigit(s[j]))
            return false;
    if(t>1440||t<0)return false;
    now.setTime_from(t);

    qDebug()<<"3:"<<now.getNumber()<<" "<<now.getName()<<" "<<now.getTime_from()<<" "<<now.getTime_to()<<" "<<now.getMax_weight();

    i+=2;
    j=i;
    for(;i<n;i++)
    {
        if(s[i]==' ')
        {
            i--;
            break;
        }
    }
    if(i==n)return false;
    t=0;
    try
    {
        t=stoi(s.substr(j,i-j+1));
    }
    catch(...)
    {
        return false;
    }
    for(;j<=i;j++)
        if(!isdigit(s[j]))
            return false;
    if(t>1440||t<0)return false;
    now.setTime_to(t);
    qDebug()<<"4:"<<now.getNumber()<<" "<<now.getName()<<" "<<now.getTime_from()<<" "<<now.getTime_to()<<" "<<now.getMax_weight();
    i+=2;
    j=i;

    unsigned long long w=0;
    try
    {
        w=stoull(s.substr(j,n-j));
    }
    catch(...)
    {
        return false;
    }
    for(;j<n;j++)
        if(!isdigit(s[j]))
            return false;

    now.setMax_weight(w);
    qDebug()<<"5:"<<now.getNumber()<<" "<<now.getName()<<" "<<now.getTime_from()<<" "<<now.getTime_to()<<" "<<now.getMax_weight();
    if(now.getTime_from()>now.getTime_to())
    {
        //qDebug()<<"Time from>Time to";
        return false;
    }
    c.push_back(now);
    select_couriers.push_back(QString::fromStdString(now.get_in_string()));
    qDebug()<<"FINAL:"<<now.getNumber()<<" "<<now.getName()<<" "<<now.getTime_from()<<" "<<now.getTime_to()<<" "<<now.getMax_weight();
    return true;
}

void MainWindow::FileSave()
{
    std::ofstream fout(file);
    fout<<orders_head<<'\n'<<orders_in_file<<couriers_head<<'\n'<<couriers_in_file;
}

void MainWindow::Clear()
{
    select_orders.clear();
    select_couriers.clear();
    order_position_in_file.clear();
    courier_position_in_file.clear();
    o.clear();
    c.clear();
    ui->CourierNumberSelect->clear();
    ui->OrderNumberSelect->clear();

}

void MainWindow::FileUpdate(long long pos, bool del)
{
    FILE *f=fopen(file.c_str(),"r+");

    qDebug()<<"{UPDATE}";
     fseek(f,pos,SEEK_SET);
    if(del)
    {
    pos-=2;
    if(pos>orders_in_file.size()+orders_head.size())
    {

        qDebug()<<couriers_in_file;
        qDebug()<<pos-orders_in_file.size()<<" "<<couriers_in_file.substr(pos-orders_in_file.size()-couriers_head.size()-orders_head.size());
        fwrite(couriers_in_file.substr(pos-orders_in_file.size()-couriers_head.size()-orders_head.size()).c_str(),sizeof(char),
               sizeof(char)*couriers_in_file.substr(pos-orders_in_file.size()-couriers_head.size()-orders_head.size()).size(),f);
    }
    else
    {
        pos-=(-1+orders_head.size());
        std::string s=orders_in_file.substr(pos)+couriers_head+"\n"+couriers_in_file;
        qDebug()<<s;
        fwrite(s.c_str(),sizeof(char),sizeof(char)*s.size(),f);
    }
    }
    else
    {
        if(pos>orders_in_file.size()+orders_head.size())
        {

            qDebug()<<couriers_in_file;
            std::string tow=couriers_in_file.substr(pos-orders_in_file.size()-couriers_head.size()-orders_head.size()-2);
            qDebug()<<pos-orders_in_file.size()<<" "<<tow;
            fwrite(tow.c_str(),sizeof(char),sizeof(char)*tow.size(),f);
        }
        else
        {
            pos-=orders_head.size();
            pos-=1;
            std::string s=orders_in_file.substr(pos)+couriers_head+"\n"+couriers_in_file;
            qDebug()<<s;
            fwrite(s.c_str(),sizeof(char),sizeof(char)*s.size(),f);
        }
    }
    fclose(f);

}

bool MainWindow::CheckCourierForm()
{
    //Check number
    std::string number=ui->NumberCurier->toPlainText().toUtf8().constData();
    if(number.size()==0)return false;
    for(auto i:number)
    {
        if(!isdigit(i))
        {
            qDebug()<<"Number has not-digit symbol";
            return false;
        }
    }

    //Check name
    std::string name=ui->NameCurier->toPlainText().toUtf8().constData();
    if(name.size()==0)return false;
    bool fl=false;
    for(auto i:name)
    {
        if(i!=' '&&i!='\n')
        {
            fl=true;
            break;
        }
    }
    if(!fl)
    {
        qDebug()<<"Bad name";
        return false;
    }

    //Check from time
    std::string t=ui->FromCurier->toPlainText().toUtf8().constData();
    if(t.size()==0)return false;
    int from=0;
    try
    {
        from=stoi(t);
    }
    catch(...)
    {
        return false;
    }
    for(auto i:t)
        if(!isdigit(i))
            return false;
    if(from>1440||from<0)return false;

    //Check to time
    t=ui->ToCurier->toPlainText().toUtf8().constData();
    if(t.size()==0)return false;
    int to=0;
    try
    {
        to=stoi(t);
    }
    catch(...)
    {
        return false;
    }
    for(auto i:t)
        if(!isdigit(i))
            return false;
    if(to>1440||to<0)return false;

    std::string weight=ui->MaxWeight->toPlainText().toUtf8().constData();
    if(weight.size()==0)return false;
    unsigned long long w=0;
    try
    {
        w=stoull(weight);
    }
    catch(...)
    {
        return false;
    }
    for(auto i:weight)
        if(!isdigit(i))
            return false;
    if(from>to)return false;

    short int tf=from,tt=to;
    if(tf>tt)
        return false;
    c_from_form={number,name,tf,tt,w};
    return true;
}

void MainWindow::FileAdd(long long pos, std::string s)
{
    FILE *f=fopen(file.c_str(),"r+");

    qDebug()<<"{Add}";
    fseek(f,pos,SEEK_SET);
    if(pos>orders_in_file.size()+orders_head.size())
    {
        qDebug()<<pos<<" "<<s;
        fwrite(s.c_str(),sizeof(char),sizeof(char)*s.size(),f);
    }
    else
    {
        pos-=(orders_head.size());
        std::string s=orders_in_file.substr(pos)+couriers_head+'\n'+couriers_in_file;
        qDebug()<<s;
        fwrite(s.c_str(),sizeof(char),sizeof(char)*s.size(),f);
    }
    fclose(f);
}

bool MainWindow::CheckOrderForm()
{
    //Check number
    std::string number=ui->NumberOrder->toPlainText().toUtf8().constData();
    if(number.size()==0)return false;
    for(auto i:number)
    {
        if(!isdigit(i))
        {
            qDebug()<<"Number has not-digit symbol";
            return false;
        }
    }
    qDebug()<<"Number:OK";
    //Check addres
    std::string addres=ui->AddresOrder->toPlainText().toUtf8().constData();
    if(addres.size()==0)return false;
    bool fl=false;
    for(auto i:addres)
    {
        if(i!=' '&&i!='\n')
        {
            fl=true;
            break;
        }
    }
    if(!fl)
    {
        qDebug()<<"Bad addres";
        return false;
    }

    qDebug()<<"Addres:OK";
    //Check from time
    std::string t=ui->TimeFromOrder->toPlainText().toUtf8().constData();
    if(t.size()==0)return false;
    int from=0;
    try
    {
        from=stoi(t);
    }
    catch(...)
    {
        return false;
    }
    for(auto i:t)
        if(!isdigit(i))
            return false;
    //qDebug()<<"TF";
    if(from>1440||from<0)return false;

    qDebug()<<"From time:OK";
    //Check to time
    t=ui->TimeToOrder->toPlainText().toUtf8().constData();
    qDebug()<<"to time = "<<t;
    if(t.size()==0)return false;
    int to=0;
    try
    {
        to=stoi(t);
    }
    catch(...)
    {
        return false;
    }
    for(auto i:t)
        if(!isdigit(i))
            return false;
    if(to>1440||to<0)return false;
       if(from>to)return false;
    qDebug()<<"To time:OK";
    std::string weight=ui->Weight->toPlainText().toUtf8().constData();
    if(weight.size()==0)return false;
    unsigned long long w=0;
    try
    {
        w=stoull(weight);
    }
    catch(...)
    {
        return false;
    }
    for(auto i:weight)
        if(!isdigit(i))
            return false;
    qDebug()<<"Weight:OK";
    //qDebug()<<"TT";
    Data df,dt;
    if(!df.Init(ui->DataFrom->toPlainText().toUtf8().constData()))
        return false;
       qDebug()<<"Data from:OK";
    if(!dt.Init(ui->DataTo->toPlainText().toUtf8().constData()))
        return false;
       qDebug()<<"Data to:OK";
    //qDebug()<<"QQ";
    if(dt>=df)
    {
           qDebug()<<"Good datas";
    }
    else
        return false;
    short int tf=from,tt=to;
    o_from_form={number,addres,df,dt,tf,tt,w};
    return true;
}

void MainWindow::DistributeOrders()
{
    long long rows=ui->CurierOrders->rowCount();
    for(int i=0;i<rows;i++)
        ui->CurierOrders->removeRow(i);
    rows=ui->IncompleteOrders->rowCount();
    for(int i=0;i<rows;i++)
        ui->IncompleteOrders->removeRow(i);

    Sort_Curriers();

}

void MainWindow::Sort_Curriers()
{
    for_sort.clear();
    for(int i=0;i<c.size();i++)
        for_sort.push_back({c[i],i});
    int n=for_sort.size();
    for(int i=0;i<n;i++)
        for(int j=i;j>0&&for_sort[j].first<for_sort[j-1].first;j--)
        {
            swap(for_sort[j],for_sort[j-1]);
        }
    for(int i=0;i<n;i++)
        qDebug()<<"["<<i<<"]"<<for_sort[i].first.get_in_string();
}

void MainWindow::on_SaveFileButton_clicked()
{
    std::string new_file="";
    QString filters("Data files (*.inf)");
    QString defaultFilter("Data files (*.inf)");
    new_file=(QFileDialog::getSaveFileName(this,"Save File",QString::fromStdString(file),filters,&defaultFilter)).toUtf8().constData();

    qDebug()<<"Save in "<<new_file;
    int n=new_file.size(),i=n-1;

    if(new_file=="")
        return;

    for(;n-i<=4;i--)
    {
        if(new_file[i]=='.')
        {
            if(new_file.substr(i,n-i)==".inf")
                break;
            return on_SaveFileButton_clicked();
        }
    }
    if(n-i>4)return;
    file=new_file;
    ui->FileBrowser->setText(QString::fromStdString(file));
    FileSave();
}


void MainWindow::on_DeleteCourierButton_clicked()
{
    if(c.size()==0)return;
    std::string s=ui->CourierNumberSelect->currentText().toUtf8().constData();
    qDebug()<<"Try to delete:"<<s;
    int n=c.size(),i=0;
    for(;i<n;i++)
    {
        //qDebug()<<c[i].get_in_string();
        if(c[i].get_in_string()==s)break;
    }
    if(i==n)return;
    qDebug()<<"Delete ["<<i<<"]";

    c.erase(c.begin()+i,c.begin()+1+i);

    select_couriers.erase(select_couriers.begin()+i,select_couriers.begin()+i+1);

    std::string u((size_t)courier_position_in_file[i].second,' ');
    u.back()='\n';
    //qDebug()<<"Space string:"<<u;
    qDebug()<<courier_position_in_file[i].first<<" "<<courier_position_in_file[i].second;
    couriers_in_file.erase(courier_position_in_file[i].first,courier_position_in_file[i].second);
    couriers_in_file+=u;
    //qDebug()<<"UPD COURIER IN FILE:"<<couriers_in_file;
    FileUpdate(couriers_head.size()+orders_head.size()+orders_in_file.size()+courier_position_in_file[i].first+2,true);

    int j=i;
    for(;i<courier_position_in_file.size();i++)
        courier_position_in_file[i].first-=(courier_position_in_file[j].second);
    courier_position_in_file.erase(courier_position_in_file.begin()+j,courier_position_in_file.begin()+1+j);
    ui->CourierNumberSelect->removeItem(j);
    //couriers_in_file.erase(i,1);
    DistributeOrders();
}


void MainWindow::on_DeleteOrderButton_clicked()
{
    if(o.size()<=0)return;
    std::string s=ui->OrderNumberSelect->currentText().toUtf8().constData();
    qDebug()<<"Try to delete:"<<s;
    int n=o.size(),i=0;
    for(;i<n;i++)
    {
        //qDebug()<<c[i].get_in_string();
        if(o[i].get_in_string()==s)break;
    }
    if(i==n)return;
    qDebug()<<"Delete ["<<i<<"]";

    o.erase(o.begin()+i,o.begin()+1+i);

    select_orders.erase(select_orders.begin()+i,select_orders.begin()+i+1);

    std::string u((size_t)order_position_in_file[i].second,' ');
    u.back()='\n';
    //qDebug()<<"Space string:"<<u;
    qDebug()<<order_position_in_file[i].first<<" "<<order_position_in_file[i].second;
    orders_in_file.erase(order_position_in_file[i].first,order_position_in_file[i].second);
    orders_in_file+=u;
    //qDebug()<<"UPD order IN FILE:"<<orders_in_file;
    FileUpdate(orders_head.size()+order_position_in_file[i].first+1,true);

    int j=i;
    for(;i<order_position_in_file.size();i++)
        order_position_in_file[i].first-=(order_position_in_file[j].second);
    order_position_in_file.erase(order_position_in_file.begin()+j,order_position_in_file.begin()+1+j);
    ui->OrderNumberSelect->removeItem(j);
    //couriers_in_file.erase(i,1);
    DistributeOrders();
}


void MainWindow::on_CourierNumberSelect_currentIndexChanged(int index)
{
    if(c.size()==0)return;
    ui->NameCurier->setPlainText(QString::fromStdString(c[index].getName()));
    ui->NumberCurier->setPlainText(QString::fromStdString(c[index].getNumber()));
    ui->FromCurier->setPlainText(QString::fromStdString(std::to_string(c[index].getTime_from())));
    ui->ToCurier->setPlainText(QString::fromStdString(std::to_string(c[index].getTime_to())));
    ui->MaxWeight->setPlainText(QString::fromStdString(std::to_string(c[index].getMax_weight())));
    //qDebug()<<"Select "<<index;
}


void MainWindow::on_AddCourierButton_clicked()
{
    if(!CheckCourierForm())return;

    //qDebug()<<"Correct data in Courier form [index]"<<index;
    c.push_back(c_from_form);

    select_couriers.push_back(QString::fromStdString(c_from_form.get_in_string()));
    ui->CourierNumberSelect->addItem(select_couriers.back());

    std::string c_string="\n"+c_from_form.get_in_file_format()+"\n";
    //qDebug()<<c_string;
    //qDebug()<<"OLD:"<<couriers_in_file;
    std::string new_cif=couriers_in_file+c_string;

    //qDebug()<<"NEW CIF:"<<new_cif;

    courier_position_in_file.push_back({couriers_in_file.size()+1,c_from_form.get_in_file_format().size()+1});
    couriers_in_file=new_cif;
     //qDebug()<<"TRY TO UPDATE";
     FileUpdate(couriers_head.size()+orders_head.size()+orders_in_file.size()+courier_position_in_file.back().first,false);
    DistributeOrders();
}


void MainWindow::on_AddOrderButton_clicked()
{
    if(!CheckOrderForm())return;

    o.push_back(o_from_form);

    select_orders.push_back(QString::fromStdString(o_from_form.get_in_string()));
    ui->OrderNumberSelect->addItem(select_orders.back());

    std::string o_string="\n"+o_from_form.get_in_file_format()+"\n";

    std::string new_cif=orders_in_file+o_string;


    order_position_in_file.push_back({orders_in_file.size()+1,o_from_form.get_in_file_format().size()+1});
    orders_in_file=new_cif;
    //qDebug()<<"TRY TO UPDATE";
    FileUpdate(orders_head.size()+order_position_in_file.back().first,false);
    DistributeOrders();
}


void MainWindow::on_ClearCourierForm_clicked()
{
    ui->NameCurier->clear();
    ui->NumberCurier->clear();
    ui->FromCurier->clear();
    ui->ToCurier->clear();
    ui->MaxWeight->clear();
}


void MainWindow::on_ClearOrderForm_clicked()
{
    ui->NumberOrder->clear();
    ui->AddresOrder->clear();
    ui->TimeFromOrder->clear();
    ui->TimeToOrder->clear();
    ui->DataFrom->clear();
    ui->DataTo->clear();
    ui->Weight->clear();
}


void MainWindow::on_OrderNumberSelect_currentIndexChanged(int index)
{
    if(o.size()==0)return;
    ui->NumberOrder->setPlainText(QString::fromStdString(o[index].get_number()));
    ui->AddresOrder->setPlainText(QString::fromStdString(o[index].get_addres()));
    //ui->TimeFromOrder->setPlainText(QString::fromStdString(o[index].get_time_from_in_string()));
    ui->TimeFromOrder->setPlainText(QString::fromStdString(std::to_string(o[index].getTime_from())));
    //ui->TimeToOrder->setPlainText(QString::fromStdString(o[index].get_time_to_in_string()));
    ui->TimeToOrder->setPlainText(QString::fromStdString(std::to_string(o[index].getTime_to())));
    ui->DataFrom->setPlainText(QString::fromStdString(o[index].get_date_from()));
    ui->DataTo->setPlainText(QString::fromStdString(o[index].get_date_to()));
    ui->Weight->setPlainText(QString::fromStdString(std::to_string(o[index].get_weight())));
}


void MainWindow::on_EditCourierInfoButton_clicked()
{
    if(!CheckCourierForm())return;

    int index=ui->CourierNumberSelect->currentIndex();
    qDebug()<<"Correct data in Courier form [index]"<<index;
    c[index]=(c_from_form);


    select_couriers[index]=(QString::fromStdString(c_from_form.get_in_string()));
    ui->CourierNumberSelect->setItemText(index,select_couriers[index]);

    std::string c_string=c_from_form.get_in_file_format()+"\n";
    qDebug()<<c_string;
    qDebug()<<"OLD:"<<couriers_in_file;
    std::string new_cif=couriers_in_file.substr(0,courier_position_in_file[index].first)+c_string+
                          couriers_in_file.substr(courier_position_in_file[index].first+courier_position_in_file[index].second);

    qDebug()<<"NEW CIF:"<<new_cif;
   long long delta=courier_position_in_file[index].second-c_string.size();
    for(int i=0;i<delta;i++)
       new_cif+=' ';
    if(delta>0)
    new_cif.back()='\n';
    couriers_in_file=new_cif;


    courier_position_in_file[index].second=c_string.size();

    for(int i=index+1;i<courier_position_in_file.size();i++)
    {
        courier_position_in_file[i].first-=delta;
    }

    //qDebug()<<"TRY TO UPDATE";
    FileUpdate(couriers_head.size()+orders_head.size()+orders_in_file.size()+courier_position_in_file[index].first+2,false);
    DistributeOrders();
}


void MainWindow::on_EditOrderInfoButton_clicked()
{
    if(!CheckOrderForm())return;

    int index=ui->OrderNumberSelect->currentIndex();
    //qDebug()<<"Correct data in order form [index]"<<index;
    o[index]=(o_from_form);


    select_orders[index]=(QString::fromStdString(o_from_form.get_in_string()));
    ui->OrderNumberSelect->setItemText(index,select_orders[index]);

    std::string o_string=o_from_form.get_in_file_format()+"\n";
    //qDebug()<<c_string;
    //qDebug()<<"OLD:"<<orders_in_file;
    std::string new_cif=orders_in_file.substr(0,order_position_in_file[index].first)+o_string+
                          orders_in_file.substr(order_position_in_file[index].first+order_position_in_file[index].second);

    qDebug()<<"NEW CIF:"<<new_cif;
    long long delta=order_position_in_file[index].second-o_string.size();
    for(int i=0;i<delta;i++)
        new_cif+=' ';
    if(delta>0)
        new_cif.back()='\n';
    orders_in_file=new_cif;


    order_position_in_file[index].second=o_string.size();

    for(int i=index+1;i<order_position_in_file.size();i++)
    {
        order_position_in_file[i].first-=delta;
    }

    FileUpdate(orders_head.size()+order_position_in_file[index].first+1,false);
    DistributeOrders();
}

