import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { CenterTipsComponent } from './center-tips.component';

describe('CenterTipsComponent', () => {
  let component: CenterTipsComponent;
  let fixture: ComponentFixture<CenterTipsComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ CenterTipsComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(CenterTipsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
