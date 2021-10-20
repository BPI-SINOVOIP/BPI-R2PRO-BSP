import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { RightBottomTipsComponent } from './right-bottom-tips.component';

describe('RightBottomTipsComponent', () => {
  let component: RightBottomTipsComponent;
  let fixture: ComponentFixture<RightBottomTipsComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ RightBottomTipsComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(RightBottomTipsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
