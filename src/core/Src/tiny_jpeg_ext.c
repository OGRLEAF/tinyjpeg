#include "tiny_jpeg_ext.h"


void print_app0(AppInfo *app_info) {
    TRACE_INFO("APP0 Application specific");
    TRACE_INFO("\t\t- format: %s", app_info->format);
    TRACE_INFO("\t\t- mVer: %d, sVer: %d", app_info->main_ver, app_info->sub_ver);
    TRACE_INFO("\t\t- unit: %d, x_den: %d, y_den: %d", app_info->unit, app_info->x_den, app_info->y_den);
    TRACE_INFO("\t\t- thumb: x: %d, y: %d", app_info->thumb_x, app_info->thumb_y);
}

void print_app2(App2Info *app2)
{
    TRACE_INFO("APP2 Application specific");
    TRACE_INFO("\t\t- APP2 Length: %d", app2->len);
}

void print_sof0(SOF0Info * sof_info)
{
    TRACE_INFO("SOF0 Start of Frame");
    TRACE_INFO("\t\t- Accur: %d", sof_info->accur);
    TRACE_INFO("\t\t- Height: %d, Width: %d", sof_info->height, sof_info->width);
    TRACE_INFO("\t\t- Comps: %d", sof_info->comp);

    for(int i=0;i<sof_info->comp;i++){
        SOF0Comp * sof_comp = sof_info->comps + i;
        TRACE_INFO("\t\t- Comp Id: %d, sample: H:V=%.2x, dqt: %d", sof_comp->comId, sof_comp->sample, sof_comp->dqt_no);
    }
}


void print_dht(DHTInfo * dht_info)
{
    TRACE_INFO("DHT Define Huffman Tables");
    TRACE_INFO("\t\t- DHT#%d, %cC", dht_info->info&0xf, dht_info->info>>4&1?'A':'D');
    TRACE_INFO_START();
    TRACE_CONTENT("\t\t- BitTable: ");

    int sum = 0;
    for(int i=0;i<16;i++){
        byte t = dht_info->bit_table[i];
        TRACE_CONTENT("%02x ", t);
        sum += t;
    };
    TRACE_CONTENT("\t\t (sum: %d)", sum);
    TRACE_END();

    TRACE_INFO_START();
    TRACE_CONTENT("\t\t- ValueTable: ");

    for(int i=0;i<sum;i++){
        byte t = dht_info->value_table[i];
        TRACE_CONTENT("%02x ", t);
    };
    TRACE_CONTENT("\t\t (sum: %d)", sum);
    TRACE_END();

    HuffLookupTable *table = &dht_info->huff_table;
    for(int i=0;i<table->size;i++)
    {
        word code = table->code[i];
        byte len = table->len[i];
        byte weight = table->weight[i];
        TRACE_INFO("\t\t+ %0.2d\t" BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN "(%d) (%d)\t",
                                i,
                                BYTE_TO_BINARY(code>>8), BYTE_TO_BINARY(code),
                                len,
                                weight);
    }
}

void print_dht_set(DHTSet * dht_set)
{
    for(int i=0;i<dht_set->n;i++)
    {
        print_dht(dht_set->dht + i);
    }
}

void print_sos(SOSInfo * sos_info)
{
    TRACE_INFO("SOS Start Of Scan");
    TRACE_INFO("\t\t- Comps: %d", sos_info->comps_n);
    for(int i=0;i<sos_info->comps_n;i++) {
        SOSComp * comp = sos_info->comps + i;
        byte a = comp->HTid>>4;
        byte b = comp->HTid & 0x0f;
        TRACE_INFO("\t\t- CompId:%d\tAC:%d DC:%d", comp->id, a, b);
    }
}

void print_dqt(DQTTable * dqt)
{
    TRACE_INFO("DQT Define Quantization Table");
    byte qt_prec = dqt->info>>4;
    byte qt_id = dqt->info & 0x0f;
    int qt_size = 64 * (qt_prec?2: 1);
    TRACE_INFO("\t\t- DQT precious: %d id: %d", qt_prec, qt_id);
    
    TRACE_INFO("\t\t- INDEX\tVALUE");
    for(int i=0;i<qt_size;i+=1)
    {
        TRACE_INFO("\t\t  %d\t%.4f", i, dqt->table[i]);
    }
}

void print_dqt_set(DQTSet * dqt_set)
{
    for(int i=0;i<dqt_set->n;i++)
    {
        print_dqt(dqt_set->dqt + i);
    }
}

void print_jpeg_struct(JPEG * jpeg)
{
    TRACE_INFO("Print jpeg structure.");
    print_app0(&jpeg->app0);
    print_app2(&jpeg->app2);
    print_dqt_set(&jpeg->dqt);

    print_sof0(&jpeg->sof0);

    print_dht_set(&jpeg->dc_dht);
    print_dht_set(&jpeg->ac_dht);


    print_sos(&jpeg->sos);
}
