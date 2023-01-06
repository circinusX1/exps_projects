#include "canvas.h"

#if WITH_GRAPH

canvas::canvas(int w, int h):_w(w),_h(h)
{
    ::memset(_graph,0,sizeof(_graph));
    ::memset(_relay,0,sizeof(_relay));
}

void    canvas::draw(String& page)
{
    page += "<hr>";
    page += F("\n<canvas width='1000' height='400' id='ka'>\n");
    page += F("\n<script>\n");
    //////////////////////////////////////////////////////////////////////////////////////
    page += F("var Scale=0;\n");
    page += F("var c=document.getElementById(\"ka\");\n");
    page += F("c.addEventListener('click', on_click, false);\n");
    page += F("var ctx=c.getContext(\"2d\");\n");
    page += F("var gw=288.0;\n");   // 5 mins in a day
    page += F("var gh=100.0;\n");
    page += F("var w=c.width;\n");
    page += F("var h=c.height;\n");

    //////////////////////////////////////////////////////////////////////////////////////
    page += F("function sx(x){return (x*w)/gw;}\n");
    page += F("function sy(y){return (y*h)/gh;}\n");

    page += F("var TriggerTemp=")+String(_trigger.t) + F(";\n");
    page += F("var TriggerHum=")+String(_trigger.h) + F(";\n");

    ///////////////////////////////////////////////////////////////////////////////////////
    page += F("var TimeNow=");
    page += String(_minutes);
    page += F(";\n");
    page += F("var GraphTemp = [");
    for(int i=0;i<SAMPLES_PER_DAY;i++)
    {
        page += String(int(_graph[i].t)) + F(",");
    }
    page += F("0];\n\n");
    page += F("var GraphHum = [");
    for(int i=0;i<SAMPLES_PER_DAY;i++)
    {
        page += String(int(_graph[i].h)) + F(",");
    }
    page += F("0];\n\n");
    page += F("var Relay = [");
    for(int e = 0; e < SAMPLES_PER_DAY;e++)
    {
        page += String(int(get_bit(e)));
        page += F(",");
    }
    page += F("0];\n\n");

    page += F("start_timer();\n");

    /////////////////////////////////////////////////////////////////////////////////////
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
              //  time,Relay,Temp,Hum,
              "fetch('/fetch', options)\n"
              "      .then(response => response.text())\n"
              "      .then(response => {\n"
              "         const svals   = response.split(\":\");\n"
              "         if(svals.length>=4){"
              "             TimeNow = parseInt(svals[0]/")+String(TIME_STEP)+F(");\n");
    page+=  F("             Relay[TimeNow]=parseInt(svals[1]);\n"
              "             GraphTemp[TimeNow]=parseInt(svals[2]);\n"
              "             GraphHum[TimeNow]=parseInt(svals[3]);\n"
              "             draw();\n"
              "         };\n"
              "  });\n"
              "}\n");
            /////////////////////////////////////////////////////////////////////////////////////

    page += F("draw();\n");
    page += F("update();\n");
    page += F("\nfunction on_click(event){;}");

    ////////////////////////////////////////////////////////////////////////////////////
    page += F("\nfunction draw()\n{\n");
    page += F("ctx.beginPath();\n");
    page += F("ctx.rect(0, 0, w, h);\n");
    page += F("ctx.fillStyle = \"#000\";\n");
    page += F("ctx.fill(); \n");

    page += F("ctx.resetTransform();\n");
    page += F("ctx.scale(1, -1);\n");
    page += F("ctx.translate(0,-h);\n");

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

    //  CURTIME LINE
    page += F("ctx.beginPath();\n");
    page += F("ctx.strokeStyle = \"#4A4\";\n");
    page += F("ctx.moveTo(sx(TimeNow)+1,0);\n");
    page += F("ctx.lineTo(sx(TimeNow)+1,h);\n");
    page += F("ctx.stroke();\n");

    //trigger lines
    page += F("ctx.beginPath();\n");
    page += F("ctx.strokeStyle = \"#F88\";\n");
    page += F("ctx.moveTo(0,sy(TriggerTemp));\n");
    page += F("ctx.lineTo(w,sy(TriggerTemp));\n");
    page += F("ctx.stroke();\n");

    page += F("ctx.beginPath();\n");
    page += F("ctx.strokeStyle = \"#88F\";\n");
    page += F("ctx.moveTo(0,sy(TriggerHum));\n");
    page += F("ctx.lineTo(w,sy(TriggerHum));\n");
    page += F("ctx.stroke();\n");

    // the sensor data points
        page += F("ctx.beginPath();\n");
        page += F("ctx.strokeStyle = \"#FAA\";\n");
        page += F("var xo=0;\n");
        page += F("var yo=sy(GraphTemp[0]);\n");
        page += F("for(var x=1; x < GraphTemp.length-1; x++)\n");
        page += F("{\n");
        page += F("    ctx.moveTo(sx(xo), sy(yo));\n");
        page += F("    ctx.lineTo(sx(x), sy(GraphTemp[x]));\n");
        page += F("    xo=x;\n");
        page += F("    yo=GraphTemp[x];\n");
        page += F("}\n");
        page += F("ctx.stroke();\n");

        page += F("ctx.beginPath();\n");
        page += F("ctx.strokeStyle = \"#AAF\";\n");
        page += F("var xo=0;\n");
        page += F("var yo=sy(GraphHum[0]);\n");
        page += F("for(var x=1; x < GraphHum.length-1; x++)\n");
        page += F("{\n");
        page += F("    ctx.moveTo(sx(xo), sy(yo));\n");
        page += F("    ctx.lineTo(sx(x), sy(GraphHum[x]));\n");
        page += F("    xo=x;\n");
        page += F("    yo=GraphHum[x];\n");
        page += F("}\n");
        page += F("ctx.stroke();\n");

//relay
        page += F("ctx.beginPath();\n");
        page += F("ctx.strokeStyle = \"#FFF\";\n");
        page += F("var xo=0;\n");
        page += F("var yo=sy(Relay[0]);\n");
        page += F("var yi=sy(Relay[1]);\n");
        page += F("for(var x=1; x < Relay.length-1; x++)\n");
        page += F("{\n");
        page += F("    ctx.moveTo(sx(xo), yo ? sy(h/10) : 0);\n");
        page += F("    ctx.lineTo(sx(x), yi ? sy(h/10) : 0);\n");
        page += F("    xo=x;\n");
        page += F("    yo=Relay[x];\n");
        page += F("    yi=Relay[x+1];\n");
        page += F("}\n");
        page += F("ctx.stroke();\n");

    page += F("ctx.resetTransform();\n");
    // hours text
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
    page += F("const tstr = GraphTemp[TimeNow].toString()+'`C';\n");
    page += F("ctx.fillText(tstr, sx(TimeNow)+5, h-sy(GraphTemp[TimeNow]) );");

    page += F("ctx.fillStyle = \"#88F\";\n");
    page += F("const hstr = GraphHum[TimeNow].toString()+'%%';\n");
    page += F("ctx.fillText(hstr, sx(TimeNow)+5, h-sy(GraphHum[TimeNow]) );");

    page += F("}\n");

    page += F("</script>\n\n");

}
#endif
