#include "typedef.h"
#include "asm/clock.h"
#include "asm/adc_api.h"
#include "timer.h"
#include "init.h"
#include "asm/efuse.h"
#include "irq.h"
#include "asm/power/p33.h"
#include "asm/power_interface.h"

u32 adc_sample(u32 ch);
static volatile u16 _adc_res;
static volatile u16 cur_ch_value;
static u8 cur_ch = 0;
struct adc_info_t {
    u32 ch;
    u16 value;
};

#define     ENABLE_OCCUPY_MODE 1

static struct adc_info_t adc_queue[ADC_MAX_CH + ENABLE_OCCUPY_MODE];

static u16 vbg_adc_value;
static u16 vbat_adc_value;

#define     ADC_SRC_CLK clk_get("adc")

/*config adc clk according to sys_clk*/
static const u32 sys2adc_clk_info[] = {
    128000000L,
    96000000L,
    72000000L,
    48000000L,
    24000000L,
    12000000L,
    6000000L,
    1000000L,
};

u32 adc_add_sample_ch(u32 ch)
{
    u32 i = 0;
    for (i = 0; i < ADC_MAX_CH; i++) {
        /* printf("%s() %d %x %x\n", __func__, i, ch, adc_queue[i].ch); */
        if (adc_queue[i].ch == ch) {
            break;
        } else if (adc_queue[i].ch == -1) {
            adc_queue[i].ch = ch;
            adc_queue[i].value = 1;
            printf("add sample ch %x\n", ch);
            break;
        }
    }
    return i;
}
u32 adc_remove_sample_ch(u32 ch)
{
    u32 i = 0;
    for (i = 0; i < ADC_MAX_CH; i++) {
        if (adc_queue[i].ch == ch) {
            adc_queue[i].ch = -1;
            break;
        }
    }
    return i;
}
static u32 adc_get_next_ch(u32 cur_ch)
{
    for (int i = cur_ch + 1; i < ADC_MAX_CH; i++) {
        if (adc_queue[i].ch != -1) {
            return i;
        }
    }
    return 0;
}
u32 adc_get_value(u32 ch)
{
    if (ch == AD_CH_VBAT) {
        return vbat_adc_value;
    }

    if (ch == AD_CH_LDOREF) {
        return vbg_adc_value;
    }

    for (int i = 0; i < ADC_MAX_CH; i++) {
        if (adc_queue[i].ch == ch) {
            return adc_queue[i].value;
        }
    }
    return 0;
}

#define     CENTER 1168
#define 	TRIM_MV	3.2f
u32 adc_value_to_voltage(u32 adc_vbg, u32 adc_ch_val)
{

    u32 adc_res = adc_ch_val;
    u32 adc_trim = get_vbg_trim();
    u32 tmp, tmp1;

    tmp1 = adc_trim & 0x1f;
    tmp = (adc_trim & BIT(5)) ? CENTER - tmp1 * TRIM_MV : CENTER + tmp1 * TRIM_MV;
    adc_res = adc_res * tmp / adc_vbg;
    return adc_res;
}


u32 adc_get_voltage(u32 ch)
{
#ifdef CONFIG_FPGA_ENABLE
    return 1000;
#endif

#define     CENTER 1168
    u32 adc_vbg = adc_get_value(AD_CH_LDOREF);
    u32 adc_res = adc_get_value(ch);


    u32 adc_trim = get_vbg_trim();
    u32 tmp, tmp1;

    tmp1 = adc_trim & 0x1f;
    tmp = (adc_trim & BIT(5)) ? CENTER - tmp1 * 3.2 : CENTER + tmp1 * 3.2;
    adc_res = adc_res * tmp / adc_vbg;


    /* printf("\n\n vbg %d\n",  adc_get_value(AD_CH_LDOREF));    */
    /* printf("%x VBAT:%d %d mv\n\n", adc_trim,                  */
    /*         adc_get_value(AD_CH_VBAT), adc_res * 4);          */
    return adc_res;
}

u32 adc_check_vbat_lowpower()
{
    u32 vbat = adc_get_value(AD_CH_VBAT);
    return __builtin_abs(vbat - 255) < 5;
}

void adc_audio_ch_select(u32 ch)
{
    SFR(JL_ANA->DAA_CON0, 12, 4, ch);
}

void adc_close()
{
    JL_ADC->CON = 0;
    JL_ADC->CON = 0;
}
void adc_suspend()
{
    JL_ADC->CON &= ~BIT(4);
}
void adc_resume()
{
    JL_ADC->CON |= BIT(4);
}

void adc_enter_occupy_mode(u32 ch)
{
    if (JL_ADC->CON & BIT(4)) {
        return;
    }
    adc_queue[ADC_MAX_CH].ch = ch;
    cur_ch_value = adc_sample(ch);
}
void adc_exit_occupy_mode()
{
    adc_queue[ADC_MAX_CH].ch = -1;
}
u32 adc_occupy_run()
{
    if (adc_queue[ADC_MAX_CH].ch != -1) {
        while (1) {
            asm volatile("idle");//wait isr
            if (_adc_res != (u16) - 1) {
                break;
            }
        }
        if (_adc_res == 0) {
            _adc_res ++;
        }
        adc_queue[ADC_MAX_CH].value = _adc_res;
        _adc_res = cur_ch_value;
        return adc_queue[ADC_MAX_CH].value;
    }
    return 0;
}
u32 adc_get_occupy_value()
{
    if (adc_queue[ADC_MAX_CH].ch != -1) {
        return adc_queue[ADC_MAX_CH].value;
    }
    return 0;
}
u32 get_adc_div(u32 src_clk)
{
    u32 adc_clk;
    u32 adc_clk_idx;
    u32 cnt;
    adc_clk = src_clk;
    cnt = ARRAY_SIZE(sys2adc_clk_info);
    for (adc_clk_idx = 0; adc_clk_idx < cnt; adc_clk_idx ++) {
        if (adc_clk > sys2adc_clk_info[adc_clk_idx]) {
            break;
        }
    }

    if (adc_clk_idx < cnt) {
        adc_clk_idx = cnt - adc_clk_idx;
    } else {
        adc_clk_idx = cnt - 1;
    }
    return adc_clk_idx;
}


___interrupt
static void adc_isr()
{
    _adc_res = JL_ADC->RES;

    adc_pmu_ch_select(AD_CH_WVDD >> 16);
    local_irq_disable();
    JL_ADC->CON = BIT(6);
    JL_ADC->CON = 0;
    local_irq_enable();
}

u32 adc_sample(u32 ch)
{
    const u32 tmp_adc_res = _adc_res;
    _adc_res = (u16) - 1;

    u32 adc_con = 0;
    SFR(adc_con, 0, 3, 0b110);//div 96

    adc_con |= (0xf << 12); //启动延时控制，实际启动延时为此数值*8个ADC时钟
    adc_con |= (adc_queue[0].ch & 0xf) << 8;
    adc_con |= BIT(3);
    adc_con |= BIT(6);
    adc_con |= BIT(5);//ie

    SFR(adc_con, 8, 4, ch & 0xf);

    if ((ch & 0xffff) == AD_CH_PMU) {
        adc_pmu_ch_select(ch >> 16);
    } else if ((ch & 0xffff) == AD_CH_AUDIO) {
        adc_audio_ch_select(ch >> 16);
    }

    JL_ADC->CON = adc_con;
    JL_ADC->CON |= BIT(4);//en
    JL_ADC->CON |= BIT(6);//kistart

    return tmp_adc_res;
}


#define     VBG_VBAT_SCAN_CNT    10000
void adc_scan(void *priv)
{
    static u16 vbg_vbat_cnt = VBG_VBAT_SCAN_CNT;
    static u16 vbg_vbat_step = 0;
    static u16 old_adc_res;
    static u16 tmp_vbg_adc_value;

//    printf("%s() %x\n",__func__,JL_ADC->CON);
    if (adc_queue[ADC_MAX_CH].ch != -1) {//occupy mode
        return;
    }

    if (JL_ADC->CON & BIT(4)) {
        return ;
    }

    /* if (!(JL_ADC->CON & BIT(4))) { //adc disable */
    /*     return;                                  */
    /* }                                            */

    vbg_vbat_cnt ++;
    if (vbg_vbat_cnt > VBG_VBAT_SCAN_CNT) {
        if (vbg_vbat_step == 0) {
            vbg_vbat_step = 1;
            old_adc_res = _adc_res;
            adc_sample(AD_CH_LDOREF);
            return;
        } else if (vbg_vbat_step == 1) {
            vbg_vbat_step = 2;
            tmp_vbg_adc_value = adc_sample(AD_CH_VBAT);
            //printf("vbg = %d\n", tmp_vbg_adc_value);
            return;
        } else if (vbg_vbat_step == 2) {
            vbg_vbat_step = 0;
            vbat_adc_value = _adc_res;
            vbg_adc_value = tmp_vbg_adc_value;
            //printf("vbg = %d  vbat = %d\n", vbg_adc_value, vbat_adc_value);
            _adc_res = old_adc_res;
            vbg_vbat_cnt = 0;
        }
    }

    u8 next_ch;

    next_ch = adc_get_next_ch(cur_ch);

    adc_queue[cur_ch].value = adc_sample(adc_queue[next_ch].ch);

    cur_ch = next_ch;
}

//获取当前采集ad的通道总数
u8 get_cur_total_ad_ch(void)
{
    u8 total_ch = 0;
    u8 i = 0;
    while (i < ADC_MAX_CH) {
        if (adc_queue[i].ch != -1) {
            total_ch++;
        }
        /* printf("i:%d,ch:%x\n",i,adc_queue[i].ch); */
        i++;
    }
    /* printf("total_ch:%d\n",total_ch); */
    return total_ch;
}

void _adc_init(u32 sys_lvd_en)
{
    memset(adc_queue, 0xff, sizeof(adc_queue));

    JL_ADC->CON = 0;
    JL_ADC->CON = 0;

    adc_pmu_detect_en(1);

    u32 i;
    vbat_adc_value = 0;
    adc_sample(AD_CH_VBAT);
    for (i = 0; i < 10; i++) {
        while (!(JL_ADC->CON & BIT(7)));
        vbat_adc_value += JL_ADC->RES;
        JL_ADC->CON |= BIT(6);
    }
    vbat_adc_value /= 10;
    printf("vbat_adc_value = %d\n", vbat_adc_value);

    vbg_adc_value = 0;
    adc_sample(AD_CH_LDOREF);
    for (i = 0; i < 10; i++) {
        while (!(JL_ADC->CON & BIT(7)));
        vbg_adc_value += JL_ADC->RES;
        JL_ADC->CON |= BIT(6);
    }
    vbg_adc_value /= 10;
    printf("vbg_adc_value = %d\n", vbg_adc_value);

    _adc_res = 1;

    request_irq(IRQ_SARADC_IDX, 0, adc_isr, 0);

    sys_s_hi_timer_add(NULL, adc_scan, 2); //2ms

    /* void adc_test();                              */
    /* sys_s_hi_timer_add(NULL, adc_test, 1000); //2ms */
}
static u8 wvdd_lev = 0;
static u32 get_wvdd_voltage()
{
    u32 vbg_value = 0;
    u32 wvdd_value = 0;

    adc_pmu_detect_en(1);
    adc_sample(AD_CH_LDOREF);
    for (int i = 0; i < 10; i++) {
        while (!(JL_ADC->CON & BIT(7))) { //wait pending
        }

        vbg_value += JL_ADC->RES;
        JL_ADC->CON |= BIT(6);
    }

    adc_sample(AD_CH_WVDD);
    for (int i = 0; i < 10; i++) {
        while (!(JL_ADC->CON & BIT(7))) { //wait pending
        }

        wvdd_value += JL_ADC->RES;
        JL_ADC->CON |= BIT(6);
    }

#define     CENTER 1168
    u32 adc_vbg = vbg_value / 10;
    u32 adc_res = wvdd_value / 10;


    u32 adc_trim = get_vbg_trim();
    u32 tmp, tmp1;

    tmp1 = adc_trim & 0x1f;
    tmp = (adc_trim & BIT(5)) ? CENTER - tmp1 * 3.2 : CENTER + tmp1 * 3.2;
    adc_res = adc_res * tmp / adc_vbg;
    /* printf("adc_res %d mv vbg:%d wvdd:%d %x\n", adc_res, vbg_value / 10, wvdd_value / 10,adc_trim); */
    return adc_res;
}


static void wvdd_trim()
{
    wvdd_lev = 0;
    P33_CON_SET(P3_WLDO06_AUTO, 0, 3, wvdd_lev);
    WLDO06_EN(1);
    delay(2000);//1ms
    do {
        P33_CON_SET(P3_WLDO06_AUTO, 0, 3, wvdd_lev);
        delay(2000);//1ms * n
        if (get_wvdd_voltage() > 700) {
            break;
        }
        wvdd_lev ++;
    } while (wvdd_lev < 8);
    WLDO06_EN(0);

    printf("wvdd_lev: %d\n", wvdd_lev);

    power_set_wvdd(wvdd_lev);
}

void adc_init()
{
    JL_ANA->WLA_CON25 &= ~(BIT(19)); //fm
    JL_ANA->WLA_CON4 &= ~(BIT(6));//bt

    //audio
    JL_ANA->ADA_CON3 |= BIT(24);//F_VOUTL_TEST_EN_11v
    JL_ANA->ADA_CON3 |= BIT(25);//F_VOUTR_TEST_EN_11v
    JL_ANA->ADA_CON3 &= ~BIT(26);
    JL_ANA->ADA_CON3 &= ~BIT(27);
    JL_ANA->ADA_CON3 |= BIT(28);//DACVDD_TEST_EN_11v
    JL_ANA->ADA_CON3 |= BIT(29);//R_VOUTL_TEST_EN_11v
    JL_ANA->ADA_CON3 |= BIT(30);//R_VOUTR_TEST_EN_11v

    JL_CLOCK->PLL_CON1 &= ~BIT(18); //pll

    //trim wvdd
    wvdd_trim();
    _adc_init(1);

}
//late_initcall(adc_init);

void adc_test()
{

    /* printf("\n\n%s() chip_id :%x\n", __func__, get_chip_id()); */
    /* printf("%s() vbg trim:%x\n", __func__, get_vbg_trim());    */
    /* printf("%s() vbat trim:%x\n", __func__, get_vbat_trim());  */

    /* printf("\n\nWLA_CON0 %x\n", JL_ANA->WLA_CON0); */
    /* printf("WLA_CON9 %x\n", JL_ANA->WLA_CON9); */
    /* printf("WLA_CON10 %x\n", JL_ANA->WLA_CON10); */
    /* printf("WLA_CON21 %x\n", JL_ANA->WLA_CON21); */
    /*  */
    /* printf("ADA_CON %x\n", JL_ANA->ADA_CON3); */
    /* printf("PLL_CON1 %x\n", JL_CLOCK->PLL_CON1); */

    printf("\n%s() VBAT:%d %d mv\n\n", __func__,
           adc_get_value(AD_CH_VBAT), adc_get_voltage(AD_CH_VBAT) * 4);

}
void adc_vbg_init()
{
    return ;
}
//__initcall(adc_vbg_init);
