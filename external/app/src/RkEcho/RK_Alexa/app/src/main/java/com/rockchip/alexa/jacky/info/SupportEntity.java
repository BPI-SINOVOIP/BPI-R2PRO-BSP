package com.rockchip.alexa.jacky.info;

/**
 * Created by Administrator on 2017/3/22.
 */

public class SupportEntity {
    private int logoId;
    private int name;
    private String desc;
    private boolean toggle;
    private int rightImg;

    public int getLogoId() {
        return logoId;
    }

    public void setLogoId(int logoId) {
        this.logoId = logoId;
    }

    public int getName() {
        return name;
    }

    public void setName(int name) {
        this.name = name;
    }

    public String getDesc() {
        return desc;
    }

    public void setDesc(String desc) {
        this.desc = desc;
    }

    public boolean isToggle() {
        return toggle;
    }

    public void setToggle(boolean toggle) {
        this.toggle = toggle;
    }

    public int getRightImg() {
        return rightImg;
    }

    public void setRightImg(int rightImg) {
        this.rightImg = rightImg;
    }
}
