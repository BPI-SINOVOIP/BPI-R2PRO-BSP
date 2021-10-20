import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ClickTipComponent } from './click-tip.component';

describe('ClickTipComponent', () => {
  let component: ClickTipComponent;
  let fixture: ComponentFixture<ClickTipComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ClickTipComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ClickTipComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
