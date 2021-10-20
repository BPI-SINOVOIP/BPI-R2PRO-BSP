import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ParaSettingComponent } from './para-setting.component';

describe('ParaSettingComponent', () => {
  let component: ParaSettingComponent;
  let fixture: ComponentFixture<ParaSettingComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ParaSettingComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ParaSettingComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
