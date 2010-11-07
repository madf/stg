/*
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *    Date: 27.10.2002
 */

/*
 *    Author : Boris Mikhailenko <stg34@ua.fm>
 */


#include <string.h>

#include "antiflood.h"
#include "common.h"

//-----------------------------------------------------------------------------
int FloodIPCompare(void * p1, void * p2)
{
/*
Сравниваем два ИП в структру с антифлудом
 * */

FLOOD_NODE * n1, *n2;

n1 = (FLOOD_NODE *)p1;
n2 = (FLOOD_NODE *)p2;
printfd(__FILE__, "FloodIPCompare %X %X\n", n1->ip, n2->ip);
return n1->ip - n2->ip;
}
//-----------------------------------------------------------------------------
ANTIFLOOD::ANTIFLOOD():
floodTree(FloodIPCompare)
{
/*
Среднее наименьшее разрешенное время между пакетами. мс.
 * */

avrgTime = 500;
}
//-----------------------------------------------------------------------------
bool ANTIFLOOD::AllowIP(uint32_t ip, bool * logged)
{
/*
Проверка того не слишком ли часто приходят пакеты с указанного айпишника
 * */

BSPNODE * findNode;
FLOOD_NODE floodNode;

floodNode.ip = ip;
findNode = floodTree.Find(&floodNode);

gettimeofday(&tv, NULL);
currentTime = ((uint64_t)tv.tv_sec)*1000 + ((uint64_t)tv.tv_usec)/1000;

if (findNode == NULL)
    {
    AddNode(ip);
    printfd(__FILE__, "AddNode(%X)\n", ip);
    return true;
    }
else
    {
    printfd(__FILE__, "UpdateNodeTime(%X)\n", findNode->record);
    UpdateNodeTime((FLOOD_NODE*)(findNode->record));
    }

if (currentTime - CalcAvrgNodeTime((FLOOD_NODE*)findNode->record) < avrgTime)
    {
    if (((FLOOD_NODE*)findNode->record)->logged == false)
        {
        *logged = false;
        ((FLOOD_NODE*)findNode->record)->logged = true;
        //floodNode.logged = true;
        }
    else
        {
        *logged = true;
        }
    return false;
    }

((FLOOD_NODE*)findNode->record)->logged = false;
return true;
}
//-----------------------------------------------------------------------------
void ANTIFLOOD::UpdateNodeTime(FLOOD_NODE * node)
{
/*
Обновляем время последнего прешедшего пакета
 * */

node->timeIP[node->pos] = currentTime;

node->pos++;
if (node->pos >= FLOOD_LBL_MAX)
    node->pos = 0;

}
//-----------------------------------------------------------------------------
void ANTIFLOOD::Clean()
{
/*
Чмстка дерева со слишком старыми данными
 * */

BSPNODE * bspNode = floodTree.Max(floodTree.GetRoot());
FLOOD_NODE * n;
BSPNODE * delNode;
currentTime = ((uint64_t)tv.tv_sec)*1000 + ((uint64_t)tv.tv_usec)/1000;

while (bspNode)
    {
    n = (FLOOD_NODE*)bspNode->record;
    if (currentTime - CalcAvrgNodeTime(n) > 2 * 60)
        {
        delNode = floodTree.Delete(bspNode);
        delete (FLOOD_NODE*)delNode->record;
        delete delNode;
        }
    bspNode = floodTree.Max(floodTree.GetRoot());
    }

printfd(__FILE__, "after clean max=%X\n", floodTree.Max(floodTree.GetRoot()));
printfd(__FILE__, "after clean min=%X\n", floodTree.Min(floodTree.GetRoot()));

}
//-----------------------------------------------------------------------------
void ANTIFLOOD::AddNode(uint32_t ip)
{
/*
Добавляем новый ИП в дерево
 * */

FLOOD_NODE * fn;
BSPNODE * node;

fn = new FLOOD_NODE;

fn->pos = 0;
fn->ip = ip;
fn->logged = false;

memset(fn->timeIP, 0, sizeof(uint64_t) * FLOOD_LBL_MAX);
fn->timeIP[0] = currentTime;

node = new BSPNODE;
node->record = fn;

floodTree.Add(node);
}
//-----------------------------------------------------------------------------
uint64_t ANTIFLOOD::CalcAvrgNodeTime(FLOOD_NODE * fn)
{
/*
Вычисляем среднее время последних прешедших пакетов
 * */

uint64_t t = 0;
for (int i = 0; i < FLOOD_LBL_MAX; i++)
    t += fn->timeIP[i];

printfd(__FILE__, "node time %lld\n", t/FLOOD_LBL_MAX);
printfd(__FILE__, "current time %lld\n", currentTime);

return t/FLOOD_LBL_MAX;
}
//-----------------------------------------------------------------------------
void ANTIFLOOD::SetAvrgTime(uint64_t t)
{
avrgTime = t;
}
//-----------------------------------------------------------------------------


