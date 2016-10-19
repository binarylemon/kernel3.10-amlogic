ifeq ($(CONFIG_OF),y)

dtb-$(CONFIG_ARCH_MESON8)	+= meson.dtb

dtb-$(CONFIG_ARCH_MESON8B)      += \
        meson8b_m100_1G.dtb \
        meson8b_m102_1G.dtb \
        meson8b_m200_1G.dtb \
        meson8b_m201_1G.dtb \
        meson8b_m201C_512M.dtb \
        meson8b_pxp.dtb \
        meson8b_m101_512M.dtb \
        meson8b_m200_1G.dtb \
        meson8b_m200_1G.dtb \
        meson8b_m201_512M.dtb \
        meson8b_m202_512M.dtb \
        meson8b_skt.dtb \
        mesona111_a200_256m.dtb

targets += $(dtb-y)

endif

# *.dtb used to be generated in the directory above. Clean out the
# old build results so people don't accidentally use them.
dtbs: $(addprefix $(obj)/, $(dtb-y))
#	$(Q)rm -f $(obj)/*.dtb
clean-files := *.dtb
