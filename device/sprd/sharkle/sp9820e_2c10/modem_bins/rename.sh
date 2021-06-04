echo `pwd`

cp SC9600_sharkle_pubcp_customer_Feature_Phone_modem.dat SC9600_sharkle_pubcp_customer_Feature_Phone_modem_base.dat
cp SHARKLE1_DM_DSP.bin SHARKLE1_DM_DSP_base.bin
cp SharkLE_LTEA_DSP_evs_off.bin SharkLE_LTEA_DSP_evs_off_base.bin
cp sharkle_cm4.bin sharkle_cm4_base.bin
cp PM_sharkle_cm4.bin PM_sharkle_cm4_base.bin

mv SC9600_sharkle_pubcp_customer_Feature_Phone_modem_base.dat ltemodem.bin
mv SHARKLE1_DM_DSP_base.bin ltegdsp.bin
mv SharkLE_LTEA_DSP_evs_off_base.bin ltedsp.bin
mv sharkle_cm4_base.bin pmsys.bin
mv PM_sharkle_cm4_base.bin wcnmodem.bin



