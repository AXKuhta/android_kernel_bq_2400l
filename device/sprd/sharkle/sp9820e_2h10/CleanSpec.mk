# ************************************************
# NEWER CLEAN STEPS MUST BE AT THE END OF THE LIST
# ************************************************
$(call add-clean-step, rm -f $(PRODUCT_OUT)/system/build.prop)
$(call add-clean-step, rm -f $(PRODUCT_OUT)/root/default.prop)
