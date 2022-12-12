

#include "html5_c.h"

#if WITH_GRAPH
html5_c::html5_c()
{

}

void html5_c::begin(int w, int h)
{
    _w = w;
    _h = h;
    _has = 0;
}

//point in FIVE minutes
void html5_c::loop(const thp_str_t& thp, int minutes)
{
    _samples[minutes/FIVE].temp = thp.temp;
    _samples[minutes/FIVE].hum  = thp.hum;
    _samples[minutes/FIVE].pres = thp.pres;
    _samples[minutes/FIVE].relay = thp.relay;
    _minutes = minutes/FIVE;
    _thp = thp;
    _has |= thp.temp != 0.0f ? 0x01 : 0x0;
    _has |= thp.hum  >  1.0f ? 0x02 : 0x0;
    _has |= thp.pres >  0.01 ? 0x04 : 0x0;
}

void html5_c::page(String& page)
{

    page += F("\n<canvas width='700' height='380' id='ka'>\n");

    page += F("\n<script>\n");
    page += F("var Scale=0;\n");
    page += F("////////////////////////////////\n");

    page += F("////////////////////////////////\n");
    page += F("var c=document.getElementById(\"ka\");\n");
    page += F("c.addEventListener('click', on_click, false);\n");
    page += F("var ctx=c.getContext(\"2d\");\n");
    page += F("var gw=288.0;\n");   // 5 mins in a day
    page += F("var gh=100.0;\n");
    page += F("var w=c.width;\n");
    page += F("var h=c.height;\n");

    page += F("var h_limit=");
    page += String(_thp.hum_l);
    page += F(";\n");
    page += F("var t_limit=");
    page += String(_thp.temp_l);
    page += F(";\n");
    page += F("var p_limit=");
    page += String(_thp.pres_l);
    page += F(";\n");


    page += F("////////////////////////////////\n");
    page += F("function sx(x){return (x*w)/gw;}\n");
    page += F("function sy(y){return (y*h)/gh;}\n");


    page += F("////////////////////////////////\n");
    page += F("var CurTime=");
    page += String(_minutes);
    page += F(";\n");

    page += F("var Temps = [");
    if(_has&0x1)
    {
        for(int e = 0; e < DAY_OF5-1;e++)
        {
            page += String(int((_samples[e].temp)));
            page += F(",");
        }
    }
    page += F("0];\n");

    page += F("var Hums = [");
    if(_has&0x2)
    {
        for(int e = 0; e < DAY_OF5-1;e++)
        {
            page += String(int((_samples[e].hum)));
            page += F(",");
        }
    }
    page += F("0];\n");

    page += F("var Press = [");
    if(_has&0x4)
    {
        for(int e = 0; e < DAY_OF5-1;e++)
        {
            page += String(int((_samples[e].pres/20.0f)));
            page += F(",");
        }
    }
    page += F("0];\n\n");

    page += F("var Relay = [");
    for(int e = 0; e < DAY_OF5-1;e++)
    {
        page += String(int((_samples[e].relay)));
        page += F(",");
    }
    page += F("0];\n\n");

    page += F("start_timer();\n");
    page += F("//////////////////////////////\n"
              "function start_timer() {\n"
              "    setInterval(update, 5000);\n"
              "}\n\n"
              "///////////////////////////////\n"
              "function update() {\n"
              "let options = {\n"
              "  method: 'GET',\n"
              "  headers: {}\n"
              "};\n"
              "fetch('/?fetch', options)\n"
              "      .then(response => response.text())\n"
              "      .then(response => {\n"
              "         const thl   = response.split(\":\");\n"
              "         if(thl.length==5){\n"
              "             const tt = parseInt(thl[0]);\n"
              "             const hh = parseInt(thl[1]);\n"
              "             const pp = parseInt(thl[2]);\n"
              "             const rr = parseInt(thl[3]);\n"
              "             const ti = parseInt(thl[4]);\n"
              "             if(ti<Temps.length) Temps[ti]=tt;\n"
              "             if(ti<Hums.length)  Hums[ti]=hh;\n"
              "             if(ti<Press.length) Press[ti]=pp;\n"
              "             if(ti<Relay.length) Relay[ti]=rr;\n"
              "             CurTime = ti;\n"
              "             draw(Temps,Hums,Press,Relay,ti);\n"
              "         }\n"
              "  });\n"
              "}\n");

    page += F("draw(Temps,Hums,Press,Relay,CurTime);\n");
    page += F("update();\n");
    page += F("function on_click(event){if(Scale==0)Scale=3;else Scale=0; update();} ");
    page += F("////////////////////////////////\n");
    page += F("function draw(temps,hums,press,relays,curtime)\n{\n");
    page += F("ctx.beginPath();\n");
    page += F("ctx.rect(0, 0, w, h);\n");
    page += F("ctx.fillStyle = \"#000\";\n");
    page += F("ctx.fill(); \n");

    page += F("ctx.resetTransform();\n");
    page += F("ctx.scale(1, -1);\n");
    page += F("ctx.translate(0,-h);\n");

    page += F("if(Scale){;\n");
    page += F(" const left = curtime > (Scale*12) ? (curtime-(Scale*12)) : 0;\n");
    page += F(" ctx.scale(Scale ,1);\n");
    page += F(" ctx.translate(sx(-left),1);\n");

    page += F("};\n");

    // hour and trigger lines
    page += F("ctx.beginPath();\n");
        page += F("ctx.strokeStyle = \"#4A4\";\n");
        page += F("ctx.moveTo(sx(curtime)+1,0);\n");
        page += F("ctx.lineTo(sx(curtime)+1,h);\n");
    page += F("ctx.stroke();\n");

    page += F("ctx.beginPath();\n");
        page += F("ctx.strokeStyle = \"#E33\";\n");
        page += F("ctx.moveTo(0,sy(t_limit));\n");
        page += F("ctx.lineTo(w,sy(t_limit));\n");
    page += F("ctx.stroke();\n");

    page += F("ctx.beginPath();\n");
        page += F("ctx.strokeStyle = \"#33E\";\n");
        page += F("ctx.moveTo(0,sy(h_limit));\n");
        page += F("ctx.lineTo(w,sy(h_limit));\n");
    page += F("ctx.stroke();\n");

    page += F("ctx.beginPath();\n");
        page += F("ctx.strokeStyle = \"#3E3\";\n");
        page += F("ctx.moveTo(0,sy(p_limit));\n");
        page += F("ctx.lineTo(w,sy(p_limit));\n");
    page += F("ctx.stroke();\n");

    // hours and units
    page += F("ctx.beginPath();\n");
        page += F("ctx.strokeStyle = \"#446\";\n");
        page += F("for(var x=0; x < w; x += w/24)\n");
        page += F("{\n");
        page += F("    ctx.moveTo(x, 0);\n");
        page += F("    ctx.lineTo(x, h);\n");
        page += F("}\n");
        page += F("for(var yy=0; yy < h; yy+=h/10)\n");
        page += F("{\n");
        page += F("    ctx.moveTo(0, yy);\n");
        page += F("    ctx.lineTo(w, yy);\n");
        page += F("}\n");
    page += F("ctx.stroke();\n");

    if(_has&0x1)
    {
        page += F("ctx.beginPath();\n");
        page += F("ctx.strokeStyle = \"#fAA\";\n");
        page += F("var xo=0;\n");
        page += F("var yo=sy(temps[0]);\n");
        page += F("for(var x=1; x < sx(temps.length); x++)\n");
        page += F("{\n");
        page += F("    ctx.moveTo(sx(xo), sy(yo));\n");
        page += F("    ctx.lineTo(sx(x), sy(temps[x]));\n");
        page += F("    xo=x;\n");
        page += F("    yo=temps[x];\n");
        page += F("}\n");
        page += F("ctx.stroke();\n");
    }

    if(_has&0x2)
    {
        page += F("ctx.beginPath();\n");
        page += F("ctx.strokeStyle = \"#AAf\";\n");
        page += F("var xo=0;\n");
        page += F("var yo=sy(hums[0]);\n");
        page += F("for(var x=1; x < sx(hums.length); x++)\n");
        page += F("{\n");
        page += F("    ctx.moveTo(sx(xo), sy(yo));\n");
        page += F("    ctx.lineTo(sx(x), sy(hums[x]));\n");
        page += F("    xo=x;\n");
        page += F("    yo=hums[x];\n");
        page += F("}\n");
        page += F("ctx.stroke();\n");
    }


    if(_has&0x4)
    {
        page += F("ctx.beginPath();\n");
        page += F("ctx.strokeStyle = \"#9F9\";\n");
        page += F("var xo=0;\n");
        page += F("var yo=sy(press[0]);\n");
        page += F("for(var x=1; x < sx(press.length); x++)\n");
        page += F("{\n");
        page += F("    ctx.moveTo(sx(xo), sy(yo));\n");
        page += F("    ctx.lineTo(sx(x), sy(press[x]));\n");
        page += F("    xo=sx;\n");
        page += F("    yo=press[x];\n");
        page += F("}\n");
        page += F("ctx.stroke();\n");
    }

        page += F("ctx.beginPath();\n");
        page += F("ctx.strokeStyle = \"#FF9\";\n");
        page += F("var xo=0;\n");
        page += F("var yo=sy(relays[0]);\n");
        page += F("for(var x=1; x < sx(relays.length); x++)\n");
        page += F("{\n");
        page += F("    ctx.moveTo(sx(xo), sy(yo));\n");
        page += F("    ctx.lineTo(sx(x), relays[x] ? h/20 : 0);\n");
        page += F("    xo=sx;\n");
        page += F("    yo=relays[x];\n");
        page += F("}\n");
        page += F("ctx.stroke();\n");

    page += F("ctx.resetTransform();\n");
    // hours
    page += F("var hour = 0;\n");
    page += F("ctx.font = \"12px Arial\";\n");
    page += F("ctx.fillStyle = \"#888\";\n");
    page += F("for(var x=0; x < w; x += w/24)\n");
    page += F("{\n");
    page += F(" const ss = hour.toString();\n");
    page += F(" ctx.fillText(ss, x+2, 14);\n");
    page += F(" hour++;\n");
    page += F("}\n");

    page += F("hour = 0;\n");
    page += F("ctx.fillStyle = \"#888\";\n");
    page += F("for(var y=0; y < h; y += h/10)\n");
    page += F("{\n");
    page += F(" const ss = hour.toString();\n");
    page += F(" ctx.fillText(ss, 1, h-y);\n");
    page += F(" hour+=10;\n");
    page += F("}\n");
    // temp

    page += F("ctx.fillStyle = \"#F88\";\n");
    page += F("const tstr = Temps[curtime].toString()+'`C';\n");
    page += F("ctx.fillText(tstr, sx(curtime)+5, h-sy(Temps[curtime]) );");

    page += F("ctx.fillStyle = \"#88F\";\n");
    page += F("const hstr = Hums[curtime].toString()+'%%';\n");
    page += F("ctx.fillText(hstr, sx(curtime)+5, h-sy(Hums[curtime]) );");

    page += F("}\n");

    page += F("</script>\n\n");


}
#endif
